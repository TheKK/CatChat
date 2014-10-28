/*
 * CatChat
 * Copyright (C) 2014 TheKK <thumbd03803@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* ===================== Headers ===================== */
#include <getopt.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>

#include "connect.h"
#include "mediaManager.h"

/* ===================== Macros ===================== */
#define MAX_NAME_SIZE 20

#define TRY_OR_EXIT(cmd); \
	do { \
		if (cmd != 0) { \
			perror(#cmd); \
			exit(EXIT_FAILURE); \
		} \
	} while(0);

#define TRY_OR_RETURN(cmd); \
	do { \
		if (cmd != 0) { \
			perror(#cmd); \
			return -1; \
		} \
	} while(0);

/* ===================== Global variables ===================== */
char* sock_path;
pthread_t sender_t, receiver_t;
sem_t client_shouldDie;
sem_t client_connected;

/* ===================== Prototypes ===================== */
void sig_handler(int signum, siginfo_t* info, void* ptr);

/* ===================== Functions ===================== */
void
client_showVersion()
{
	printf("chatchat client (OS homework) 0.01\n");
}

void
client_showHelp()
{
	printf("chatchat client: [-d socket dir][-v version][-h help]\n");
}

void
client_getopt(int argc, char* argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, "d:p:vh")) != -1) {
		switch (opt) {
		case 'd':
			sock_path = optarg;
			break;
		case 'p':
			printf("WIP\n");
			break;
		case 'v':
			client_showVersion();
		case 'h':
		case '?':
		default:
			client_showHelp();
			break;
		}
	}
}

int
client_init()
{
	struct sigaction act;
	char mdName[20];

	sender_t = -1;
	receiver_t = -1;

	/* Setup signal handler */
	act.sa_sigaction = sig_handler;
	act.sa_flags = SA_RESTART;
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGPIPE, &act, NULL);

	srand(time(NULL));
	snprintf(mdName, sizeof(mdName), ".client%d", (rand() % 99999));
	TRY_OR_RETURN(mdManager_init(mdName));

	/* Init semaphore */
	TRY_OR_RETURN(sem_init(&client_shouldDie, 0, 0));
	TRY_OR_RETURN(sem_init(&client_connected, 0, 0));

	/* Initialize socket stuffs */
	printf("\r[SYSTEM]Init...\n");
	TRY_OR_RETURN(cnct_Init(AF_LOCAL, sock_path));

	return 0;
}

void
client_quit()
{
	if (sender_t != -1)
		pthread_cancel(sender_t);

	if (receiver_t != -1)
		pthread_cancel(receiver_t);

	mdManager_quit();

	sem_destroy(&client_shouldDie);
	sem_destroy(&client_connected);

	printf("\r[SYSTEM]Quit...\n");
	TRY_OR_EXIT(cnct_Quit());
}


int
client_getServerAnswer()
{
	char msg[10];

	/* Wait for server to check if we are allow to connect */
	cnct_RecvMsg(cnct_GetSocket(), msg);
	if (strcmp(msg, "y") == 0)
		return 1;
	else if (strcmp(msg, "n") == 0)
		return 0;
}

/* TODO maybe place this to other file */
void
remove_next_line_symbol(char* str)
{
	char* c;
	c = index(str, '\n');
	if (c == NULL)
		return;
	else
		*c = '\0';
}

void
cilent_doCmd(char* cmd)
{
	if (strcmp(cmd, "q") == 0 || strcmp(cmd, "quit") == 0)
		sem_post(&client_shouldDie);
	else if (strcmp(cmd, "h") == 0|| strcmp(cmd, "help") == 0)
		printf("Help page work in progress\n");
	else
		printf("[SYSTEM] Command %s not found\n", cmd);
}

/* ===================== Thread functions ===================== */
void*
thread_sender(void* args)
{
	char msg[CONNECT_MAX_MSG_SIZE + 1];

	/* Tell server you name */
	while (1) {
		printf("[SYSTEM]What is you name: ");
		fgets(msg, MAX_NAME_SIZE, stdin);
		remove_next_line_symbol(msg);

		cnct_SendMsg(cnct_GetSocket(), msg);
		cnct_RecvMsg(cnct_GetSocket(), msg);
		if (strcmp(msg, "y") == 0)
			break;
		else if (strcmp(msg, "n") == 0) {
			printf("[SYSTEM]This name is been uesed, "
			       "use another\n");
		}
	}
	sem_post(&client_connected);

	while (1) {
		fgets(msg, CONNECT_MAX_MSG_SIZE, stdin);
		remove_next_line_symbol(msg);

		if (msg[0] == ':') /* Is command */
			cilent_doCmd(msg + 1);
		else if (msg[0] != '\0') /* Not empty text */
			if (cnct_SendMsg(cnct_GetSocket(), msg) < 0) {
				perror("cnct_SendMsg()");
				sem_post(&client_shouldDie);
				pthread_exit(NULL);
			}
	}

	return  NULL;
}

void*
thread_receiver(void* args)
{
	char msg[CONNECT_MAX_MSG_SIZE];
	int flag;

	/* Wait for other thread to finish registration */
	sem_wait(&client_connected);

	while (1) {
		flag = cnct_RecvMsg(cnct_GetSocket(), msg);

		if (flag == -1) {
			perror("cnct_RecvMsg()");
			sem_post(&client_shouldDie);
			break;
		} else if (flag == 0) { /* Disconnected */
			perror("cnct_RecvMsg()");
			sem_post(&client_shouldDie);
			break;
		}

		printf("\r%s\n", msg);
	}

	return NULL;
}

/* ===================== Signal handler ===================== */
void
sig_handler(int signum, siginfo_t* info, void* ptr)
{
	static int count = 0;

	if (signum == SIGPIPE) {
		printf("\r[SYSTEM]Disconnected...\n");
		TRY_OR_EXIT(cnct_Quit());
		exit(EXIT_FAILURE);
	}

	printf("\r[SYSTEM]Use \":q<enter>\" to exit\n");

	if (count++ == 7) {
		printf("\rTeaching you, is hard...(noise)\n");
		printf("\rSo you know what? You win, just go...(beep)\n");
		sem_post(&client_shouldDie);
	}
}

/* ===================== Main function ===================== */
int
main(int argc, char* argv[])
{
	client_getopt(argc, argv);
	if (sock_path == NULL) {
		client_showHelp();
		return 1;
	}

	TRY_OR_EXIT(client_init());

	printf("\r[SYSTEM]Connecting...\n");
	TRY_OR_EXIT(cnct_Connect());

	/* Connected  then wait for death... */
	printf("\r[SYSTEM]Connected!!\n");
	if (client_getServerAnswer() == 1) {
		pthread_create(&sender_t, NULL, (void*) thread_sender, NULL);
		pthread_create(&receiver_t, NULL, (void*) thread_receiver,
			       NULL);
		sem_wait(&client_shouldDie);
	}

	/* Clean these mess and quit */
	client_quit();

	return EXIT_SUCCESS;
}

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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "connect.h"

/*
 * C-Thread-Pool
 * Author:  Johan Hanssen Seferidis
 * Created: 2011-08-12
 *
 * License: LGPL
 */
#include "thpool.h"

/* ===================== Macros ===================== */
#define LISTEN_BACKLOG 5
#define SERVER_POOL_SIZE 3

#define TRY(cmd); \
	do { \
		if (cmd == -1) { \
			perror(#cmd); \
			exit(EXIT_FAILURE); \
		} \
	} while(0);

/* ===================== Global variables ===================== */
static sem_t server_shouldDie;
struct thpool_t* thpool;

struct job_args
{
	int fd;
};

/* ===================== Prototypes ===================== */
void sig_handler(int signum, siginfo_t* info, void* ptr);

/* ===================== Functions ===================== */
void
server_showVersion()
{
	printf("chatchat server (OS homework) 0.01\n");
}

void
server_showHelp()
{
	printf("chatchat server: [-d socket dir][-v version][-h help]\n");
}

/* ===================== Thread functions ===================== */
void*
thread_clientHandler(void* args)
{
	/* TODO: User char pointer rather array of char */
	char name[20];
	char recvMsg[CONNECT_MAX_MSG_SIZE + 1];
	char sendMsg[sizeof(name) + sizeof(" say: ") + sizeof(recvMsg)];
	int flag;
	int peer_fd;

	peer_fd = ((struct job_args*)args)->fd;

	/* Receive name from client */
	cnct_RecvMsg(peer_fd, name);
	printf("\r[INFO]client %s connected: peer_fd = %d\n", name, peer_fd);

	while (1) {
		flag = cnct_RecvMsg(peer_fd, recvMsg);

		/* Check error */
		if (flag == -1) {
			perror("recv()");
			break;
		} else if (flag == 0) {
			printf("\r[INFO]client disconnected: fd = %d\n",
			       peer_fd);
			break;
		}

		/* Check if you cast the spell your grandma told you... */
		if (strcmp(recvMsg, "palus") == 0) {
			printf("My eye!!! My EYEEEEEEE!!!\n");
			sem_post(&server_shouldDie);
			break;
		}

		memset(sendMsg, 0, sizeof(sendMsg));
		strcat(sendMsg, name);
		strcat(sendMsg, " say: ");
		strcat(sendMsg, recvMsg);
		cnct_SendMsg(peer_fd, sendMsg);
		printf("\r[MSG]%s (fd = %d) said: %s\n",
		       name, peer_fd, sendMsg);
	}

	return NULL;
}

void*
thread_accepter(void* args)
{
	int peer_fd;
	struct job_args job_args;

	while (1) {
		peer_fd = cnct_Accept((struct sockaddr*) NULL, NULL);
		if (peer_fd == -1) {
			perror("cnct_Accept()");
			break;
		}
		else {
			job_args.fd = peer_fd;
			thpool_add_work(thpool,
					thread_clientHandler, (void*)&job_args);
		}
	}

	return NULL;
}

/* ===================== Signal handler ===================== */
void
sig_handler(int signum, siginfo_t* info, void* ptr)
{
	sem_post(&server_shouldDie);
}

/* ===================== Main function ===================== */
int
main(int argc, char* argv[])
{
	char* sock_path;
	int opt;
	pthread_t accepter_t;
	struct sigaction act;

	/* Set option */
	while ((opt = getopt(argc, argv, "d:vh")) != -1) {
		switch (opt) {
		case 'd':
			sock_path = optarg;
			break;
		case 'v':
			server_showVersion();
		case 'h':
		case '?':
		default:
			server_showHelp();
			break;
		}
	}
	if (sock_path == NULL) {
		server_showHelp();
		return EXIT_FAILURE;
	}

	/* Setup signal handler */
	act.sa_sigaction = sig_handler;
	act.sa_flags = SA_RESTART;
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);

	/* Init pool */
	thpool = thpool_init(SERVER_POOL_SIZE);

	/* Init semaphore */
	sem_init(&server_shouldDie, 0, 0);

	/* Initialization socket stuffs */
	printf("\r[SYSTEM]Init...\n");
	TRY(cnct_Init(AF_LOCAL, sock_path));

	printf("\r[SYSTEM]Binding...\n");
	TRY(cnct_Bind());

	printf("\r[SYSTEM]Listening...\n");
	TRY(cnct_Listen(LISTEN_BACKLOG));

	/* Server on then wait for death... */
	pthread_create(&accepter_t, NULL, thread_accepter, NULL);
	sem_wait(&server_shouldDie);

	/* Clean these mess and quit */
	pthread_cancel(accepter_t);
	thpool_destroy(thpool);
	sem_destroy(&server_shouldDie);

	printf("\r[SYSTEM]Quit...\n");
	TRY(cnct_Quit());
	TRY(cnct_Remove());

	return EXIT_SUCCESS;
}

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
#include <ncurses.h>

#include "connect.h"

/* ===================== Macros ===================== */
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
char name[10];
pthread_t sender_t, receiver_t;
sem_t client_shouldDie;

WINDOW* input_win;
WINDOW* dialog_win;
WINDOW* dialog_pad;
int input_win_x, input_win_y, input_win_w, input_win_h;
int dialog_win_x, dialog_win_y, dialog_win_w, dialog_win_h;
int dialog_pad_x, dialog_pad_y, dialog_pad_w, dialog_pad_h;

/* ===================== Prototypes ===================== */
void sig_handler(int signum, siginfo_t* info, void* ptr);
void remove_next_line_symbol(char* str);

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

	sender_t = -1;
	receiver_t = -1;

	/* Setup signal handler */
	act.sa_sigaction = sig_handler;
	act.sa_flags = SA_RESTART;
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGPIPE, &act, NULL);

	/* Init semaphore */
	TRY_OR_RETURN(sem_init(&client_shouldDie, 0, 0));

	/* Initialize socket stuffs */
	printf("\r[SYSTEM]Init...\n");
	TRY_OR_RETURN(cnct_Init(AF_LOCAL, sock_path));

	return 0;
}

int
client_getServerAnswer()
{
	char msg[10];
	/* Wait for server */
	cnct_RecvMsg(cnct_Getfd(), msg);
	if (strcmp(msg, "boo") != 0)
		return 0;

	/* Tell server you name */
	printf("[SYSTEM]What is you name: ");
	fgets(msg, 10, stdin);
	remove_next_line_symbol(msg);
	cnct_SendMsg(cnct_Getfd(), msg);
	strncat(name, msg, sizeof(name));

	return 1;
}

void
client_quit()
{
	if (sender_t != -1)
		pthread_cancel(sender_t);

	if (receiver_t != -1)
		pthread_cancel(receiver_t);
	sem_destroy(&client_shouldDie);

	printf("\r[SYSTEM]Quit...\n");
	TRY_OR_EXIT(cnct_Quit());
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


	while (1) {
		/* Show prefix */
		/* FIXME: Cursor cant' locate after prefix symbol */
		mvwprintw(input_win, 1, 1, " <%s> ", name);
		wrefresh(input_win);

		wgetstr(input_win, msg);
		werase(input_win);
		box(input_win, 0, 0);
		wrefresh(input_win);

		remove_next_line_symbol(msg);

		if (msg[0] == ':') /* Is command */
			cilent_doCmd(msg + 1);
		else if (msg[0] != '\0') /* Not empty text */
			if (cnct_SendMsg(cnct_Getfd(), msg) < 0) {
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

	while (1) {
		flag = cnct_RecvMsg(cnct_Getfd(), msg);

		if (flag == -1) {
			perror("cnct_RecvMsg()");
			sem_post(&client_shouldDie);
			break;
		} else if (flag == 0) { /* Disconnected */
			perror("cnct_RecvMsg()");
			sem_post(&client_shouldDie);
			break;
		}

		/* printf("\r%s\n", msg); */
		wprintw(dialog_pad, "%s\n", msg);
		prefresh(dialog_pad, 0, 0, 1, 1, dialog_pad_h, dialog_pad_w);
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

void
ninit()
{
		initscr();
		refresh();

		dialog_win = newwin(LINES - 3, COLS - 1, 0, 0);
		box(dialog_win, 0, 0);
		wrefresh(dialog_win);
		getbegyx(dialog_win, dialog_win_y, dialog_win_x);
		getmaxyx(dialog_win, dialog_win_h, dialog_win_w);

		input_win = newwin(3, COLS - 1, LINES - 3, 0);
		box(input_win, 0, 0);
		wrefresh(input_win);
		getbegyx(input_win, input_win_y, input_win_x);
		getmaxyx(input_win, input_win_h, input_win_w);

		dialog_pad_x = dialog_win_x + 1;
		dialog_pad_y = dialog_win_y + 1;
		dialog_pad_w = dialog_win_w - 2;
		dialog_pad_h = dialog_win_h - 2;

		dialog_pad = newpad(dialog_pad_h, dialog_pad_w);

		prefresh(dialog_pad, 0, 0, 1, 1, dialog_pad_h, dialog_pad_w);
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
		ninit();
		pthread_create(&sender_t, NULL, (void*) thread_sender, NULL);
		pthread_create(&receiver_t, NULL, (void*) thread_receiver,
			       NULL);
		sem_wait(&client_shouldDie);
	}

	/* Clean these mess and quit */
	client_quit();

	{
		delwin(dialog_win);
		delwin(dialog_pad);
		delwin(input_win);

		endwin();
	}


	return EXIT_SUCCESS;
}

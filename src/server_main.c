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
static char* sock_path;
static bool server_is_running = true;
static struct thpool_t* thpool;

struct job_args
{
	int fd;
};

/* ===================== Functions ===================== */
void
show_version()
{
	printf("chatchat server (OS homework) 0.01\n");
}

void
show_help()
{
	printf("chatchat server: [-d socket dir][-v version][-h help]\n");
}

void
set_opt(int argc, char* argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, "d:vh")) != -1) {
		switch (opt) {
		case 'd':
			sock_path = optarg;
			break;
		case 'v':
			show_version();
		case 'h':
		case '?':
		default:
			show_help();
			break;
		}
	}
}

/* ===================== Thread functions ===================== */
void*
client_handler(void* args)
{
	char msg[MAX_TEXT_SIZE];
	char* name = NULL;
	int peer_fd;
	int flag;

	peer_fd = ((struct job_args*)args)->fd;

	printf("\r[INFO]client connected: peer_fd = %d\n", peer_fd);

	while (1) {
		flag = cnct_RecvMsg(peer_fd, msg);

		/* Check error */
		if (flag == -1) {
			perror("recv()");
			break;
		} else if (flag == 0) {
			printf("\r[INFO]client disconnected: fd = %d\n",
			       peer_fd);
			break;
		}

		/* Check if you cast the spell */
		if (strcmp(msg, "palus") == 0) {
			printf("My eye!!! My EYEEEEEEE!!!\n");
			server_is_running = false;
			break;
		}

		cnct_SendMsg(peer_fd, msg);
		printf("\r[MSG]%s (fd = %d) said: %s\n",
		       name, peer_fd, msg);
	}

	return NULL;
}

void*
accepter(void* args)
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
					client_handler, (void*)&job_args);
		}
	}

	return NULL;
}

/* ===================== Signal handler ===================== */
void
sig_handler(int signum, siginfo_t* info, void* ptr)
{
	server_is_running = false;
}

/* ===================== Main function ===================== */
int
main(int argc, char* argv[])
{
	struct sigaction act;
	pthread_t accepter_t;

	/* Setup signal handler */
	act.sa_sigaction = sig_handler;
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);

	set_opt(argc, argv);
	if (sock_path == NULL) {
		show_help();
		return 1;
	}

	/* Initialization */
	printf("\r[SYSTEM]Init...\n");
	TRY(cnct_Init(AF_LOCAL, sock_path));

	printf("\r[SYSTEM]Binding...\n");
	TRY(cnct_Bind());

	printf("\r[SYSTEM]Listening...\n");
	TRY(cnct_Listen(LISTEN_BACKLOG));

	/* Init pool */
	thpool = thpool_init(SERVER_POOL_SIZE);

	/* Server on */
	pthread_create(&accepter_t, NULL, accepter, NULL);

	while (server_is_running);

	pthread_cancel(accepter_t);

	/* Quit */
	printf("\r[SYSTEM]Quit...\n");
	TRY(cnct_Quit());
	TRY(cnct_Remove());
	thpool_destroy(thpool);

	return EXIT_SUCCESS;
}

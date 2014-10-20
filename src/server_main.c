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
#include "C-Thread-Pool/thpool.h"

/* ===================== Macros ===================== */
#define LISTEN_BACKLOG 5

#define TRY(cmd); \
	do { \
		if (cmd == -1) { \
			perror(#cmd); \
			exit(EXIT_FAILURE); \
		} \
	} while(0);

/* ===================== Global variables ===================== */
char* sock_path;
bool server_is_running = true;

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

void*
client_handler(void* args)
{
	char msg[MAX_TEXT_SIZE];
	char* name = NULL;
	int fd;
	int flag;

	printf("\r[INFO]client connected: peer_fd = %d\n", fd);

	while (server_is_running) {
		flag = cnct_RecvMsg(fd, msg);
		if (flag == -1) {
			perror("recv()");
			break;
		} else if (flag == 0) {
			printf("\r[INFO]client disconnected: fd = %d\n",
			       fd);
			break;
		}
		printf("\r[MSG]%s (fd = %d) said: %s\n",
		       name, fd, msg);
	}

	pthread_exit(NULL);
}

void
sig_handler(int signum, siginfo_t* info, void* ptr)
{
	server_is_running = false;
}

int
main(int argc, char* argv[])
{
	int id, peer_fd;
	struct sockaddr_un peer_addr;
	socklen_t peer_addrlen;
	struct sigaction act;

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
	TRY(clientPool_Init(client_handler));

	printf("\r[SYSTEM]Binding...\n");
	TRY(cnct_Bind());

	printf("\r[SYSTEM]Listening...\n");
	TRY(cnct_Listen(LISTEN_BACKLOG));

	while (server_is_running) {
		while (clientPool_IsFull());

		clientPool_GetID(&id);
		peer_fd = cnct_Accept((struct sockaddr*) &peer_addr,
				      &peer_addrlen);

		if (peer_fd == -1)
			break;
		else {
			/*pthread_create(&client_list.thread[client_list.size],*/
				       /*NULL, client_handler, &arg);*/
		}
	}

	/* Quit */
	printf("\r[SYSTEM]Quit...\n");
	TRY(cnct_Quit());
	TRY(cnct_Remove());
	TRY(clientPool_Quit());

	return EXIT_SUCCESS;
}

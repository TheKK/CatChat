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
#include "userlist.h"

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
#define MAX_USER_COUNT 2

#define TRY_OR_EXIT(cmd); \
	do { \
		if (cmd == -1) { \
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
pthread_t accepter_t;
static pthread_mutex_t sendMutex = PTHREAD_MUTEX_INITIALIZER;
static sem_t server_shouldDie;
static thpool_t* thpool;
static userlist_list_t* userlist;

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

void
server_getopt(int argc, char* argv[])
{
	int opt;

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
}

int
server_init()
{
	struct sigaction act;

	/* Setup signal handler */
	act.sa_sigaction = sig_handler;
	act.sa_flags = SA_RESTART;
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);

	/* Init pool and user list */
	thpool = thpool_init(MAX_USER_COUNT);
	userlist = userlist_create(MAX_USER_COUNT);

	/* Init semaphore */
	TRY_OR_RETURN(sem_init(&server_shouldDie, 0, 0));

	/* Initialization socket stuffs */
	printf("\r[SYSTEM]Init...\n");
	TRY_OR_RETURN(cnct_Init(AF_LOCAL, sock_path));

	printf("\r[SYSTEM]Binding...\n");
	TRY_OR_RETURN(cnct_Bind());

	printf("\r[SYSTEM]Listening...\n");
	TRY_OR_RETURN(cnct_Listen(LISTEN_BACKLOG));

	return 0;
}

void
server_quit()
{
	/* Clean these mess and quit */
	pthread_cancel(accepter_t);
	thpool_forceDestroy(thpool);
	userlist_destroy(userlist);
	pthread_mutex_destroy(&sendMutex);
	sem_destroy(&server_shouldDie);

	printf("\r[SYSTEM]Quit...\n");
	TRY_OR_EXIT(cnct_Quit());
	TRY_OR_EXIT(cnct_Remove());
}

void
server_setNewUser(int fd, char name[USERLIST_MAX_NAME_SIZE + 1])
{
	/* Receive name from client */
	cnct_RecvMsg(fd, name);
	printf("\r[INFO]client %s connected: peer_fd = %d\n", name, fd);

	userlist_add(userlist, fd, name);
}

/* ===================== Thread functions ===================== */
void*
thread_clientHandler(void* args)
{
	/* TODO: User char pointer rather array of char */
	char name[USERLIST_MAX_NAME_SIZE + 1];
	char recvMsg[CONNECT_MAX_MSG_SIZE + 1];
	char sendMsg[sizeof(name) + sizeof(" say: ") + sizeof(recvMsg)];
	int client_fd;
	int flag;
	int i;

	client_fd = ((struct job_args*)args)->fd;

	server_setNewUser(client_fd, name);

	while (1) {
		flag = cnct_RecvMsg(client_fd, recvMsg);

		/* Check error */
		if (flag == -1) {
			perror("recv()");
			break;
		} else if (flag == 0) {
			printf("\r[INFO]client disconnected: fd = %d\n",
			       client_fd);
			break;
		}

		/* Check if you cast the spell your grandma told you... */
		if (strcmp(recvMsg, "palus") == 0) {
			printf("My eye!!! My EYEEEEEEE!!!\n");
			sem_post(&server_shouldDie);
			break;
		}

		/* Command from client */
		if (strcmp(recvMsg, "/users") == 0) {
			cnct_SendMsg(client_fd, "====== User list ======");
			for (i = 0; i < userlist_getCurrentSize(userlist);
			     i++) {
				sprintf(sendMsg,"%5i %s",
					i,
					userlist_getName(userlist, i));
				cnct_SendMsg(client_fd, sendMsg);
			}
			cnct_SendMsg(client_fd, "====== End ======");
			continue;
		}

		printf("\r[MSG]%s (fd = %d) said: %s\n",
		       name, client_fd, recvMsg);

		for (i = 0; i < userlist_getCurrentSize(userlist); i++) {
			sprintf(sendMsg,"%s say: %s", name, recvMsg);
			cnct_SendMsg(userlist_getFd(userlist, i), sendMsg);
		}
	}

	userlist_remove(userlist, userlist_findByFd(userlist, client_fd));
	close(client_fd);

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
			if (userlist_isFull(userlist)) {
				printf("[SYSTEM] FULL\n");
				cnct_SendMsg(peer_fd, "no");
				close(peer_fd);
			} else {
				/* Tell client that I hear his/her call */
				cnct_SendMsg(peer_fd, "boo");
				job_args.fd = peer_fd;
				thpool_add_work(thpool, thread_clientHandler,
						(void*)&job_args);
			}
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
	server_getopt(argc, argv);
	if (sock_path == NULL) {
		server_showHelp();
		return EXIT_FAILURE;
	}

	TRY_OR_EXIT(server_init());

	/* Server on then wait for death... */
	pthread_create(&accepter_t, NULL, thread_accepter, NULL);
	sem_wait(&server_shouldDie);

	server_quit();

	return EXIT_SUCCESS;
}

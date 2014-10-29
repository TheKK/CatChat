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
#include <libintl.h>
#include <locale.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "connect.h"
#include "userlist.h"
#include "mediaManager.h"

/* ===================== Macros ===================== */
#define LISTEN_BACKLOG 5

#define FLAG_DISCONNECT	-2
#define FLAG_OTHER	-1
#define FLAG_NOERROR	0

#define TRY_OR_EXIT(cmd); \
	do { \
		if (cmd == -1) { \
			perror(#cmd); \
			exit(EXIT_FAILURE); \
		} \
	} while(0);

#define TRY_OR_RETURN(cmd); \
	do { \
		if (cmd == -1) { \
			perror(#cmd); \
			return -1; \
		} \
	} while(0);

#define _(STRING) gettext(STRING)

/* ===================== Global variables ===================== */
static char* sock_path;
static pthread_t accepter_t;
static pthread_mutex_t sendMutex = PTHREAD_MUTEX_INITIALIZER;
static sem_t server_shouldDie;
static sem_t server_threadCreated;
static userlist_list_t* userlist;
static int max_user_count;

struct job_args
{
	int socket;
};

/* ===================== Prototypes ===================== */
void sig_handler(int signum, siginfo_t* info, void* ptr);

/* ===================== Functions ===================== */
void
server_showVersion()
{
	printf(_("CatChat server (OS homework) 0.11\n"));
}

void
server_showHelp()
{
	printf(_("CatChat server Usage: [-d socket dir]"
	       "[-m max client number][-v version][-h help]\n"));
}

void
server_getopt(int argc, char* argv[])
{
	int opt;

	/* Default */
	sock_path = NULL;
	max_user_count = 2;

	while ((opt = getopt(argc, argv, "d:m:vh")) != -1) {
		switch (opt) {
		case 'd':
			sock_path = optarg;
			break;
		case 'm':
			max_user_count = atoi(optarg);
			break;
		case 'v':
			server_showVersion();
		case 'h':
		case '?':
		default:
			server_showHelp();
			exit(0);
		}
	}
}

void
server_l10nInit()
{
	/* l10n stuff */
	setlocale(LC_ALL, "");
	bindtextdomain("CatChat_server", "po");
	textdomain("CatChat_server");
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

	/* Init user list and other stuffs */
	userlist = userlist_create();
	if (!userlist) {
		fprintf(stderr, _("\r[SYSTEM] userlist_create() falied\n"));
		return 1;
	}

	TRY_OR_RETURN(mdManager_init(".server"));

	/* Init semaphore */
	TRY_OR_RETURN(sem_init(&server_shouldDie, 0, 0));
	TRY_OR_RETURN(sem_init(&server_threadCreated, 0, 0));

	/* Initialization socket stuffs */
	printf(_("\r[SYSTEM] Init system...\n"));
	TRY_OR_RETURN(cnct_Init(AF_LOCAL, sock_path));

	printf(_("\r[SYSTEM] Binding...\n"));
	TRY_OR_RETURN(cnct_Bind());

	printf(_("\r[SYSTEM] Listening...\n"));
	TRY_OR_RETURN(cnct_Listen(LISTEN_BACKLOG));

	return 0;
}

void
server_quit()
{
	/* Clean these mess and quit */
	pthread_cancel(accepter_t);
	mdManager_quit();
	userlist_destroy(userlist);
	pthread_mutex_destroy(&sendMutex);
	sem_destroy(&server_shouldDie);
	sem_destroy(&server_threadCreated);

	printf(_("\r[SYSTEM] Quit...\n"));
	TRY_OR_EXIT(cnct_Quit());
	TRY_OR_EXIT(cnct_Remove());
}

int
server_checkFlag(int flag)
{
	switch (flag) {
	case -1:
		return FLAG_OTHER;
		break;
	case 0:
		return FLAG_DISCONNECT;
		break;
	default:
		return FLAG_NOERROR;
		break;
	}
}

int
server_tellConnectPermission(int socket)
{
	if (userlist_getCurrentSize(userlist) < max_user_count) {
		cnct_SendMsg(socket, "y");
		return 1;
	} else {
		cnct_SendMsg(socket, "n");
		return 0;
	}
}

int
server_showOnlineUserTo(int socket)
{
	char sendMsg[CONNECT_MAX_MSG_SIZE];
	int i;
	userlist_info_t* userInfo;

	TRY_OR_RETURN(cnct_SendMsg(socket, _("====== User list ======")));
	for (i = 0; i < userlist_getCurrentSize(userlist); i++) {
		userInfo = userlist_findByIndex(userlist, i);
		sprintf(sendMsg,"%5i %s", i, userInfo->name);
		TRY_OR_RETURN(cnct_SendMsg(socket, sendMsg));
	}
	TRY_OR_RETURN(cnct_SendMsg(socket, _("====== End ======")));

	return 0;
}

void
server_addNewUser(int socket, pthread_t tid,
		  char name[USERLIST_MAX_NAME_SIZE + 1])
{
	/* Receive name from client */
	while (1) {
		cnct_RecvMsg(socket, name);

		/* If name already exist in current userlist */
		if (userlist_findByName(userlist, name)) {
			cnct_SendMsg(socket, "n");
		} else {
			cnct_SendMsg(socket, "y");
			break;
		}
	}

	printf(_("\r[INFO] Client %s connected: peer_socket = %d\n"),
	       name, socket);

	userlist_add(userlist, socket, tid, name);
}

int
server_sendMsgToEveryone(char* client_name, int client_sock, char* recvMsg)
{
	char sendMsg[CONNECT_MAX_MSG_SIZE + sizeof(recvMsg)];
	int i, flag;
	userlist_info_t* userInfo;

	/* Normal texts */
	printf(_("\r[CLIENT] %s(socket = %d) said: %s\n"),
	       client_name, client_sock, recvMsg);

	for (i = 0; i < userlist_getCurrentSize(userlist); i++) {
		userInfo = userlist_findByIndex(userlist, i);

		sprintf(sendMsg,"[%s] %s", client_name, recvMsg);
		flag = cnct_SendMsg(userInfo->socket, sendMsg);
		if (flag <= 0)
			return flag;
	}

	return flag;
}

int
server_doReq(char* client_name, int client_sock, char* req)
{
	char msg[CONNECT_MAX_MSG_SIZE];
	char tosend[2 * CONNECT_MAX_MSG_SIZE];
	char name[USERLIST_MAX_NAME_SIZE];
	userlist_info_t* userInfo;

	/* Command from client */
	if (strcmp(req, "users") == 0) {			/* /user */
		TRY_OR_RETURN(server_showOnlineUserTo(client_sock));
	} else if (sscanf(req, "%[^ ]%[^$]", name, msg) == 2) {	/* DM */
		userInfo = userlist_findByName(userlist, name);
		if (userInfo) {
			sprintf(tosend, _("<%s talk to you> %s\n"),
				client_name, msg);
			cnct_SendMsg(userInfo->socket, tosend);

			sprintf(tosend, _("<you talk to %s> %s\n"),
				userInfo->name, msg);
			cnct_SendMsg(client_sock, tosend);
		} else {
			cnct_SendMsg(client_sock,
				     _("[SERVER] User not found\n"));
		}
	} else {
		sprintf(msg, _("[SYSTEM] Request <\\%s> not found\n"), req);
		TRY_OR_RETURN(cnct_SendMsg(client_sock, msg));
	}

	return 0;
}

/* ===================== Thread functions ===================== */
void*
thread_clientGreeter(void* args)
{
	/* TODO: User char pointer rather array of char */
	char name[USERLIST_MAX_NAME_SIZE + 1];
	char recvMsg[CONNECT_MAX_MSG_SIZE + 1];
	int client_sock, flag, client_is_alive;
	uint32_t type;

	client_sock = ((struct job_args*)args)->socket;

	/* Tell server you've used job_args and others can use it now */
	sem_post(&server_threadCreated);

	server_addNewUser(client_sock, pthread_self(), name);

	client_is_alive = 1;
	while (client_is_alive) {
		flag = cnct_RecvRequestType(client_sock, &type);
		if (flag <= 0) {
			client_is_alive = 0;
			continue;
		}

		/* Check type */
		switch (type) {
		case CNCT_TYPE_MSG:
			flag = cnct_RecvMsg(client_sock, recvMsg);
			server_sendMsgToEveryone(name, client_sock,
						 recvMsg);
			break;
		case CNCT_TYPE_REQ:
			flag = cnct_RecvMsg(client_sock, recvMsg);
			server_doReq(name, client_sock, recvMsg);
			break;
		case CNCT_TYPE_FILE:
			break;
		}
		if (flag <= 0) {
			client_is_alive = 0;
			continue;
		}

		/* Check if you cast the spell your grandma told you... */
		if (strcmp(recvMsg, "palus") == 0) {
			printf(_("\rMy eye!!! My EYEEEEEEE!!!\n"));
			sem_post(&server_shouldDie);
			client_is_alive = 0;
			continue;
		}
	}

	printf(_("\r[INFO] Client disconnected: socket = %d\n"), client_sock);

	userlist_remove(userlist, name);
	close(client_sock);

	return NULL;
}

void*
thread_accepter(void* args)
{
	int peer_sock;
	pthread_t tid;
	struct job_args job_args;

	while (1) {
		peer_sock = cnct_Accept((struct sockaddr*) NULL, NULL);

		switch (peer_sock) {
		case -1:	/* Error */
			perror("cnct_Accept()");
			sem_post(&server_shouldDie);
			break;
		default:
			/* Tell client if he/she can join this server */
			if (!server_tellConnectPermission(peer_sock))
				break;

			job_args.socket = peer_sock;
			pthread_create(&tid, NULL, thread_clientGreeter,
				       &job_args);

			/* Make sure new thread has copied job_args */
			sem_wait(&server_threadCreated);
			break;
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
	server_l10nInit();

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

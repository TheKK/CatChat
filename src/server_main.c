/*
 * Author: KK <thumbd03803@gmail.com>
 *
 * File: server_main.c
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

struct client_list
{
	pthread_mutex_t muxtex;
	uint8_t size;
	pthread_t thread[5];
};
struct client_list client_list;

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
	int peer_fd;
	int flag;

	peer_fd = ((int*) args)[0];
	printf("\r[INFO]client connected: peer_fd = %d\n", peer_fd);

	while (server_is_running) {
		flag = cnct_RecvMsg(peer_fd, msg);
		if (flag == -1) {
			perror("recv()");
			break;
		} else if (flag == 0) {
			printf("\r[INFO]client disconnected: peer_fd = %d\n",
			       peer_fd);
			break;
		}
		printf("\r[MSG]client (peer_fd = %d) said: %s\n",
		       peer_fd, msg);
	}

	pthread_mutex_lock(&client_list.muxtex);
	client_list.size--;
	printf("\r[INFO]client count: %d\n", client_list.size);
	pthread_mutex_unlock(&client_list.muxtex);

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
	int peer_fd;
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

	pthread_mutex_init(&client_list.muxtex, NULL);
	client_list.size = 0;

	printf("\r[SYSTEM]Init...\n");
	TRY(cnct_Init(AF_LOCAL, sock_path));

	printf("\r[SYSTEM]Binding...\n");
	TRY(cnct_Bind());

	printf("\r[SYSTEM]Listening...\n");
	TRY(cnct_Listen(LISTEN_BACKLOG));

	while (server_is_running) {
		peer_fd = cnct_Accept((struct sockaddr*) &peer_addr,
				      &peer_addrlen);
		if (peer_fd == -1)
			break;
		else {
			pthread_mutex_lock(&client_list.muxtex);

			pthread_create(&client_list.thread[client_list.size],
				       NULL, client_handler, &peer_fd);
			client_list.size++;

			pthread_mutex_unlock(&client_list.muxtex);
		}
	}

	printf("\r[SYSTEM]Quit...\n");
	TRY(cnct_Quit());
	TRY(cnct_Remove());

	pthread_mutex_destroy(&client_list.muxtex);

	return EXIT_SUCCESS;
}

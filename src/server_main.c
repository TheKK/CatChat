/*
 * Author: KK <thumbd03803@gmail.com>
 *
 * File: server_main.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>

#include "connect.h"

#define LISTEN_BACKLOG 5

#define try(cmd); \
	if (cmd == -1) exit(EXIT_FAILURE);

char* sock_path;
int is_running = 1;

int peer_fd;
struct sockaddr_un peer_addr;
socklen_t peer_addrlen;

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

void
start_work()
{
	char msg[MAX_TEXT_SIZE];
	int len;

	while (is_running) {
		if (read(peer_fd, &len, sizeof(len)) == 0)
			continue;

		read(peer_fd, msg, len);
		printf("msg: %s\n", msg);
	}
}

int
main(int argc, char* argv[])
{
	set_opt(argc, argv);
	if (sock_path == NULL) {
		show_help();
		return 1;
	}

	printf("Init...\n");
	try(cnct_Init(AF_LOCAL, sock_path));

	printf("Binding...\n");
	try(cnct_Bind());

	printf("Listening...\n");
	try(cnct_Listen(LISTEN_BACKLOG));

	peer_fd = cnct_Accept((struct sockaddr*) &peer_addr, &peer_addrlen);
	printf("server socket fd: %d\n", cnct_Getfd());
	printf("client socket fd: %d\n", peer_fd);
	printf("Connection accept!\n");

	printf("Statr job\n");
	start_work();

	printf("Quit...\n");
	try(cnct_Quit());
	try(cnct_Remove());

	return 0;
}


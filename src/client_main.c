/*
 * Author: KK <thumbd03803@gmail.com>
 *
 * File: client_main.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>

#include "connect.h"

#define try(cmd); \
	if (cmd == -1) exit(EXIT_FAILURE);

char* sock_path;
int is_running = 1;

void
show_version()
{
	printf("chatchat client (OS homework) 0.01\n");
}

void
show_help()
{
	printf("chatchat client: [-d socket dir][-v version][-h help]\n");
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
do_cmd(char* cmd)
{
	switch (*cmd) {
	case 'h':
		break;
	case 'q':
		is_running = 0;
		break;
	default:
		break;
	}
}

void
start_work()
{
	char msg[MAX_TEXT_SIZE];
	int len;

	while (is_running) {
		fgets(msg, MAX_TEXT_SIZE, stdin);
		if (msg[0] == ':')
			do_cmd(msg + 1);
		else {
			len = sizeof(msg);
			write(cnct_Getfd(), &len, sizeof(len));
			write(cnct_Getfd(), msg, len);
			printf("send: %s\n", msg);
		}
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

	printf("client socket fd: %d\n", cnct_Getfd());

	printf("Connecting...\n");
	try(cnct_Connect());

	printf("Statr job\n");
	start_work();

	printf("Quit...\n");
	try(cnct_Quit());

	return 0;
}


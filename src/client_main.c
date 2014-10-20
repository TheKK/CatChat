/*
 * Author: KK <thumbd03803@gmail.com>
 *
 * File: client_main.c
 */

#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include "connect.h"

/* ===================== Macros ===================== */
#define TRY(cmd); \
	do { \
		if (cmd == -1) { \
			perror(#cmd); \
			exit(EXIT_FAILURE); \
		} \
	} while(0);

/* ===================== Global variables ===================== */
char* sock_path;
bool client_is_running = true;

/* ===================== Functions ===================== */
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
	if (strcmp(cmd, "q") == 0 || strcmp(cmd, "quit") == 0)
		client_is_running = false;
	else if (strcmp(cmd, "h") == 0|| strcmp(cmd, "help") == 0)
		printf("Help page work in progress\n");
	else
		printf("[SYSTEM] Command %s not found\n", cmd);
}

void
start_work()
{
	char msg[MAX_TEXT_SIZE];
	memset(msg, 0, MAX_TEXT_SIZE);

	while (client_is_running) {
		printf(">>");
		fgets(msg, MAX_TEXT_SIZE, stdin);
		remove_next_line_symbol(msg);
		if (msg[0] == ':')
			do_cmd(msg + 1);
		else {
			if (cnct_SendMsg(msg) != -1)
				printf("I said: %s\n", msg);
		}
	}
}

void
sig_handler(int signum, siginfo_t* info, void* ptr)
{
	printf("\r[SYSTEM]Use \":q<enter>\" to exit\n");
}

int
main(int argc, char* argv[])
{
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

	printf("Init...\n");
	TRY(cnct_Init(AF_LOCAL, sock_path));

	printf("client socket fd: %d\n", cnct_Getfd());

	printf("Connecting...\n");
	TRY(cnct_Connect());

	printf("Statr job\n");
	start_work();

	printf("Quit...\n");
	TRY(cnct_Quit());

	return 0;
}


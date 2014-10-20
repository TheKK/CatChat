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

	while (client_is_running) {
		printf(">>");

		memset(msg, 0, MAX_TEXT_SIZE);
		fgets(msg, MAX_TEXT_SIZE, stdin);
		remove_next_line_symbol(msg);

		if (msg[0] == ':') /* Is command */
			do_cmd(msg + 1);
		else if (msg[0] == '\0') /* No text */
			continue;
		else {
			TRY(cnct_SendMsg(msg));
			printf("I said: %s\n", msg);
		}
	}
}

void
sig_handler(int signum, siginfo_t* info, void* ptr)
{
	if (signum == SIGPIPE) {
		printf("\r[SYSTEM]Disconnected...\n");
		TRY(cnct_Quit());
		exit(EXIT_FAILURE);
	}

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
	sigaction(SIGPIPE, &act, NULL);

	set_opt(argc, argv);
	if (sock_path == NULL) {
		show_help();
		return 1;
	}

	printf("\r[SYSTEM]Init...\n");
	TRY(cnct_Init(AF_LOCAL, sock_path));

	printf("\r[SYSTEM]client socket fd: %d\n", cnct_Getfd());

	printf("\r[SYSTEM]Connecting...\n");
	TRY(cnct_Connect());

	printf("\r[SYSTEM]Start work\n");
	start_work();

	printf("\r[SYSTEM]Quit...\n");
	TRY(cnct_Quit());

	return EXIT_SUCCESS;
}

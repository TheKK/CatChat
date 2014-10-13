/*
 * Author: KK <thumbd03803@gmail.com>
 *
 * File: client_main.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define LISTEN_BACKLOG 50
#define MAX_MSG_LENGTH 150

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)

/* server fd, client fd */
int cfd;

char* sock_path = NULL;
struct sockaddr_un my_addr;

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

int
init()
{
	cfd = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (cfd == -1)
		handle_error("socket");

	/* Clean struct */
	memset(&my_addr, 0, sizeof(struct sockaddr_un));

	my_addr.sun_family = AF_LOCAL;
	strcpy(my_addr.sun_path, sock_path);

	if (connect(cfd, (struct sockaddr *) &my_addr,
		    sizeof(struct sockaddr_un)) == -1)
		handle_error("connect");

	return 0;
}

void
start_work()
{
	char msg[MAX_MSG_LENGTH];
	int length;

	while ((strcmp(msg, "bye\0")) != 0){
		fgets(msg, MAX_MSG_LENGTH, stdin);
		/*send(cfd, msg, sizeof(msg), 0);*/
		length = sizeof(msg);
		write(cfd, &length, sizeof(length));
		write(cfd, msg, length);
		printf("msg send: %s\n", msg);
	}
}

void
quit()
{
	close(cfd);
}

int
main(int argc, char* argv[])
{
	set_opt(argc, argv);
	if (sock_path == NULL) {
		show_help();
		return 1;
	}

	if (init() == -1)
		return 1;

	start_work();

	quit();

	return 0;
}


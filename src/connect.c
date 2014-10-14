/*
 * Author: KK <thumbd03803@gmail.com>
 *
 * File: connect.c
 */

#include "connect.h"

static int my_fd;
static struct sockaddr_un my_addr;

int
cnct_Init(int domain, char* sockPath)
{
	if ((my_fd = socket(domain, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return -1;
	}

	memset(&my_addr, 0, sizeof(struct sockaddr_un));
	my_addr.sun_family = domain;
	strcpy(my_addr.sun_path, sockPath);

	return 0;
}

int
cnct_Quit()
{
	if (close(my_fd) == -1) {
		perror("close");
		return -1;
	}

	return 0;
}

int
cnct_Bind()
{
	if (bind(my_fd, (struct sockaddr*) &my_addr,
		 sizeof(struct sockaddr_un)) == -1) {
		perror("bind");
		return -1;
	}

	return 0;
}

int
cnct_Connect()
{
	if (connect(my_fd, (struct sockaddr*) &my_addr,
		    sizeof(struct sockaddr_un)) == -1) {
		perror("connect");
		return -1;
	}

	return 0;
}

int
cnct_Listen(int backlog)
{
	if (listen(my_fd, backlog) == -1) {
		perror("listen");
		return -1;
	}

	return 0;
}

int
cnct_Accept(struct sockaddr* addr, socklen_t* len)
{
	*len = sizeof(*addr);
	if (accept(my_fd, (struct sockaddr*) addr, len) == -1) {
		perror("accept");
		return -1;
	}

	return 0;
}

int
cnct_Remove()
{
	if (remove(my_addr.sun_path) == -1) {
		perror("remove");
		return -1;
	}

	return 0;
}

int
cnct_SendMsg(const char* msg)
{
	return 0;
}

int
cnct_Getfd()
{
	return my_fd;
}

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
	my_fd = socket(domain, SOCK_STREAM, 0);
	if (my_fd == -1) {
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
	int peer_fd;

	*len = sizeof(*addr);
	peer_fd = accept(my_fd, (struct sockaddr*) addr, len);
	if (peer_fd == -1) {
		perror("accept");
		return -1;
	}

	return peer_fd;
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
cnct_SendMsg(char* msg)
{
	int flag;
	uint8_t len;

	/*
	 * Send 1 byte data of length of msg
	 * then send entire msg(include \0)
	 */
	len = strlen(msg) + 1;

	flag = send(cnct_Getfd(), &len, sizeof(len), 0);
	if (flag <= 0)
		return flag;
	flag = send(cnct_Getfd(), msg, len, 0);

	return flag;
}

int
cnct_RecvMsg(int fd, char* buf)
{
	int flag;
	uint8_t len;

	/*
	 * Read 1 byte data of length of msg
	 * then read entire msg(include \0)
	 */
	flag = recv(fd, &len, sizeof(len), 0);
	if (flag <= 0)
		return flag;
	flag = recv(fd, buf, len, 0);

	return flag;
}

int
cnct_Getfd()
{
	return my_fd;
}

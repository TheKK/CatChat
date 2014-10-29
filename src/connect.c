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

#include "connect.h"

static int g_socketFd;
static int g_domain;
static struct sockaddr_un g_sockaddr_un;
static struct sockaddr_in g_sockaddr_in;

int
cnct_Init(int domain, char* sockPath, char* ip, int port)
{
	g_domain = domain;

	g_socketFd = socket(g_domain, SOCK_STREAM, 0);
	if (g_socketFd == -1)
		return -1;

	switch (g_domain) {
	case PF_INET:
		g_sockaddr_in.sin_family = AF_INET;

		/* Default value */
		if (ip == NULL)
			inet_aton("INADDR_LOOPBACK", &(g_sockaddr_in.sin_addr));
		else
			inet_aton(ip, &(g_sockaddr_in.sin_addr));

		g_sockaddr_in.sin_port = htons(port);
		break;
	case PF_LOCAL:
		memset(&g_sockaddr_un, 0, sizeof(struct sockaddr_un));
		g_sockaddr_un.sun_family = g_domain;
		strcpy(g_sockaddr_un.sun_path, sockPath);
		break;
	}

	return 0;
}

int
cnct_Quit()
{
	if (close(g_socketFd) == -1)
		return -1;

	return 0;
}

int
cnct_Bind()
{
	switch (g_domain) {
	case PF_INET:
		if (bind(g_socketFd, (struct sockaddr*) &g_sockaddr_in,
			 sizeof(struct sockaddr_in)) == -1) {
			return -1;
		}
		break;
	case PF_LOCAL:
		if (bind(g_socketFd, (struct sockaddr*) &g_sockaddr_un,
			 sizeof(struct sockaddr_un)) == -1) {
			return -1;
		}
		break;
	}

	return 0;
}

int
cnct_Connect()
{
	switch (g_domain) {
	case PF_INET:
		if (connect(g_socketFd, (struct sockaddr*) &g_sockaddr_in,
			    sizeof(struct sockaddr_in)) == -1) {
			return -1;
		}
		break;
	case PF_LOCAL:
		if (connect(g_socketFd, (struct sockaddr*) &g_sockaddr_un,
			    sizeof(struct sockaddr_un)) == -1) {
			return -1;
		}
		break;
	}

	return 0;
}

int
cnct_Listen(int backlog)
{
	if (listen(g_socketFd, backlog) == -1)
		return -1;

	return 0;
}

int
cnct_Accept(struct sockaddr* addr, socklen_t* len)
{
	int peer_fd;

	if (addr == NULL)
		len = NULL;
	else
		*len = sizeof(*addr);

	peer_fd = accept(g_socketFd, (struct sockaddr*) addr, len);
	if (peer_fd == -1)
		return -1;

	return peer_fd;
}

int
cnct_SendMsg(int socket, char* msg)
{
	int flag;
	uint8_t len;

	/*
	 * Send 1 byte data of length of msg
	 * then send entire msg(include \0)
	 */
	len = strlen(msg) + 1;

	flag = send(socket, &len, sizeof(len), 0);
	if (flag <= 0)
		return flag;
	flag = send(socket, msg, len, 0);

	return flag;
}

int
cnct_RecvMsg(int socket, char* buf)
{
	int flag;
	uint8_t len;

	/*
	 * Read 1 byte data of length of msg
	 * then read entire msg(include \0)
	 */
	flag = recv(socket, &len, sizeof(len), 0);
	if (flag <= 0)
		return flag;
	flag = recv(socket, buf, len, 0);

	return flag;
}

int
cnct_SendFile(int socket, FILE* fd)
{
	char* buf[CONNECT_MAX_BUFFER_SIZE];
	int flag;
	uint64_t size;
	uint32_t w;

	fseek(fd, 0, SEEK_END);
	size = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	/*
	 * Send 8 byte data of size of file
	 * then send entire file
	 */
	flag = send(socket, &size, sizeof(size), 0);
	if (flag <= 0)
		return flag;

	while (size) {
		if (size > CONNECT_MAX_BUFFER_SIZE)
			w = CONNECT_MAX_BUFFER_SIZE;
		else
			w = size;

		fread(buf, 1, w, fd);
		if (ferror(fd))
			return -1;

		flag = send(socket, buf, w, 0);
		if (flag <= 0)
			return flag;

		size -= w;
	}

	return flag;
}

int
cnct_RecvFile(int socket, FILE* fd)
{
	char* buf[CONNECT_MAX_BUFFER_SIZE];
	int flag;
	uint64_t size;
	uint32_t w;

	fseek(fd, 0, SEEK_SET);

	/*
	 * Read 8 byte data of size of file
	 * then read entire file
	 */
	flag = recv(socket, &size, sizeof(size), 0);
	if (flag <= 0)
		return flag;

	while (size) {
		if (size > CONNECT_MAX_BUFFER_SIZE)
			w = CONNECT_MAX_BUFFER_SIZE;
		else
			w = size;

		flag = recv(socket, buf, w, 0);
		if (flag <= 0)
			return flag;

		fwrite(buf, 1, w, fd);
		if (ferror(fd))
			return -1;

		size -= w;
	}

	return flag;
}

int
cnct_SendRequestType(int socket, cnct_type type)
{
	int flag;

	flag = send(socket, &type, 1, 0);

	return flag;
}

int
cnct_RecvRequestType(int socket, cnct_type* type)
{
	int flag;

	flag = recv(socket, type, 1, 0);

	return flag;
}

int
cnct_GetSocket()
{
	return g_socketFd;
}

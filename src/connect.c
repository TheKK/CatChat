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

static int my_fd;
static struct sockaddr_un my_addr;

int
cnct_Init(int domain, char* sockPath)
{
	my_fd = socket(domain, SOCK_STREAM, 0);
	if (my_fd == -1)
		return -1;

	memset(&my_addr, 0, sizeof(struct sockaddr_un));
	my_addr.sun_family = domain;
	strcpy(my_addr.sun_path, sockPath);

	return 0;
}

int
cnct_Remove()
{
	if (remove(my_addr.sun_path) == -1)
		return -1;

	return 0;
}

int
cnct_Quit()
{
	if (close(my_fd) == -1)
		return -1;

	return 0;
}

int
cnct_Bind()
{
	if (bind(my_fd, (struct sockaddr*) &my_addr,
		 sizeof(struct sockaddr_un)) == -1) {
		return -1;
	}

	return 0;
}

int
cnct_Connect()
{
	if (connect(my_fd, (struct sockaddr*) &my_addr,
		    sizeof(struct sockaddr_un)) == -1) {
		return -1;
	}

	return 0;
}

int
cnct_Listen(int backlog)
{
	if (listen(my_fd, backlog) == -1)
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

	peer_fd = accept(my_fd, (struct sockaddr*) addr, len);
	if (peer_fd == -1)
		return -1;

	return peer_fd;
}

int
cnct_SendMsg(int fd, char* msg)
{
	int flag;
	uint8_t len;

	/*
	 * Send 1 byte data of length of msg
	 * then send entire msg(include \0)
	 */
	len = strlen(msg) + 1;

	flag = send(fd, &len, sizeof(len), 0);
	if (flag <= 0)
		return flag;
	flag = send(fd, msg, len, 0);

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

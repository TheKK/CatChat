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

#ifndef CONNECT_H
#define CONNECT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/un.h>

int cnct_Init(int domain, char* sockPath);
int cnct_Remove();
int cnct_Quit();

int cnct_Bind();
int cnct_Connect();
int cnct_Listen(int backlog);
int cnct_Accept(struct sockaddr* addr, socklen_t* addr_len);

int cnct_SendMsg(int fd, char* msg);
int test_send(int fd, char* msg);
int cnct_RecvMsg(int fd, char* buff);

int cnct_Getfd();

#endif /* CONNECT_H */

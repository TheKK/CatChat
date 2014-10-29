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

/* ===================== Headers ===================== */
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

/* ===================== Macros ===================== */
#define CONNECT_MAX_MSG_SIZE 512
#define CONNECT_MAX_BUFFER_SIZE 16 * 1024

/* ===================== Definitions ===================== */
typedef enum
{
	CNCT_TYPE_MSG	= 0x00, /* 1 byte */
	CNCT_TYPE_FILE	= 0x01,
	CNCT_TYPE_REQ	= 0x02
} cnct_type;

/* ===================== Functions ===================== */
int cnct_Init(int domain, char* sockPath, char* ip, int port);
int cnct_Quit();

int cnct_Bind();
int cnct_Connect();
int cnct_Listen(int backlog);
int cnct_Accept(struct sockaddr* addr, socklen_t* addr_len);

int cnct_SendMsg(int socket, char* msg);
int cnct_RecvMsg(int socket, char* buff);

int cnct_SendFile(int socket, FILE* fd);
int cnct_RecvFile(int socket, FILE* fd);

int cnct_SendRequestType(int socket, cnct_type type);
int cnct_RecvRequestType(int socket, cnct_type* type);

int cnct_GetSocket();

#endif /* CONNECT_H */

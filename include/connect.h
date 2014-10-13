/*
 * Author: KK <thumbd03803@gmail.com>
 *
 * File: connect.h
 */

#ifndef CONNECT_H
#define CONNECT_H

#include <stdio.h>
#include <errno.h>

void
print_err()
{
	switch (errno) {
	case EACCES:
		break;
	case EAFNOSUPPORT:
		break;
	case EINVAL:
		break;
	case EMFILE:
		break;
	case ENFILE:
		break;
	case ENOBUFS:
		break;
	case ENOMEM:
		break;
	case EPROTONOSUPPORT:
		break;
	default:
		break;
	}
}

#endif /* CONNECT_H */

#ifndef MOVISTARTV_SOCKETS_SOCKET_H
#define MOVISTARTV_SOCKETS_SOCKET_H

#include <sys/types.h>
#ifdef WIN32
#include<winsock2.h>
#endif

#ifdef WIN32
#pragma comment(lib,"ws2_32.lib")
#endif

#endif
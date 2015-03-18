#include "sockets/socket.h"
#include <stdio.h>

int main(int argc, char * argv[])
{
#ifdef WIN32
	WSADATA wsaData;

	if (0 != WSAStartup(MAKEWORD(2,2), &wsaData)) {
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}
#endif

#ifdef WIN32
	WSACleanup();
#endif

	printf("\nPress any key to continue...");
	getchar();
}
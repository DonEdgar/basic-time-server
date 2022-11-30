/* time_server.c */
/**
 * This time server is meant to be portable, if used on 
 * a Windows machine, it will use the appropiate headers,
 * else will use POSIX headers
 * *****************************************/
#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib") //This line may or may not be needed, depending on the compiler being used

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#endif

/****
 * For portability reasons, we are defining the following macros 
 * so that checking a socket, closing a socket and getting error codes
 * can be handled accordingly
 * ************************************************************/
#if defined(_WIN32)
#define ISVALIDSOCKET(s)  ((s) != INVALIDSOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO()  (WSAGetLastError())

#else
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int //In POSIX machines, socket is like a file descriptor, thus will return an int 
#define GETSOCKETERRNO() (errno)
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>

int 
main()
{
	/*
	 * If compiling on windows, then we need to initalize WINSOCK */
#if defined(_WIN32)
	WSADATA d;
	if (WSAStartup(MAKEWORK(2,2), &d))
	{
		fprintf(stderr, "Failed to initalize.\n");
		return 1;
	}
#endif
	/*
	 * Build the host address using struct addrinfo to hold the data, 
	 *       Then we zero out the structure and set the parameters we are looking for
	 */       
	printf("configuring local address...\n");
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; //Incase IPv6 is needed change to AF_INET6
	hints.ai_socktype = SOCK_STREAM; //Incase UDP is needed change to SOCK_DGRAM
	hints.ai_flags = AI_PASSIVE;
	
	/**
	 * Uses the getaddrinfo to fill in the bind address,
	 * Does needed error checking 
	 */
	struct addrinfo *bind_address;
	int s = getaddrinfo(0, "8080", &hints, &bind_address);
	if (s != 0)
	{
		fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}


	printf("Creating socket...\n");
	SOCKET socket_listen;
	socket_listen = socket(bind_address->ai_family,
			bind_address->ai_socktype, bind_address->ai_protocol);

	if (!ISVALIDSOCKET(socket_listen))
	{
		fprintf(stderr, "socket() failed. (%d)\n",GETSOCKETERRNO());
		return 1;
	}

	printf("Binding socket to local address...\n");
	if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen))
	{
		fprintf(stderr, "bind() failed. (%d)\n",GETSOCKETERRNO());
		return 1;
	}
	freeaddrinfo(bind_address);


        printf("Listening....\n");
	if (listen(socket_listen, 10) < 0)
	{
		fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}

	printf("Waiting for connection...\n");
	struct sockaddr_storage client_address;
	socklen_t client_len = sizeof(client_address);
	SOCKET socket_client = accept( socket_listen, 
			(struct sockaddr*) &client_address, &client_len);
	if (!ISVALIDSOCKET(socket_client))
	{
		fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}

	printf("Client is connected... ");
	char address_buffer[100];
	getnameinfo((struct sockaddr*)&client_address,
			client_len, address_buffer, sizeof(address_buffer), 0,0, NI_NUMERICHOST);

	printf("%s\n", address_buffer);

	printf("Reading request...\n");
	char request[1024];
	int bytes_recieved = recv(socket_client, request, 1024,0);

	printf("Received %d bytes.\n", bytes_recieved);

	printf("%.*s", bytes_recieved, request);

	printf("Sending response...\n");
	const char *response = 
		"HTTP/1.1 200 OK\r\n"
		"Connection: close\r\n"
		"Content-Type: text/plain\r\n\r\n"
		"Local time is: ";
	int bytes_sent = send(socket_client, response, strlen(response), 0);
	printf("Sent %d of %d bytes.\n", bytes_sent, (int)strlen(response));


	time_t timer;
	time(&timer);
	char *time_msg = ctime(&timer);
	bytes_sent = send(socket_client, time_msg, strlen(time_msg), 0);
	printf("Sent %d of %d bytes.\n", bytes_sent, (int)strlen(time_msg));

	printf("Closing connection....\n");

	CLOSESOCKET(socket_client);

#if defined(_WIN32)
	WSACleanup();
#endif

	printf("Finished.\n");

	return 0;

}


#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdbool.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include"protocol.h"
#define BUFSIZE 265

// socket/network declaration functions
int connect_inet(char *host, char *service);

int connect_inet(char *host, char *service)
{
    struct addrinfo hints, *info_list, *info;
    int sock, error;

    // look up remote host
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;  // in practice, this means give us IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // indicate we want a streaming socket

    error = getaddrinfo(host, service, &hints, &info_list);
    if (error) {
	fprintf(stderr, "error looking up %s:%s: %s\n", host, service, gai_strerror(error));
	return -1;
    }

    for (info = info_list; info != NULL; info = info->ai_next) {
	sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	if (sock < 0) continue;

	error = connect(sock, info->ai_addr, info->ai_addrlen);
	if (error) {
	    close(sock);
	    continue;
	}

	break;
    }
    freeaddrinfo(info_list);

    if (info == NULL) {
	fprintf(stderr, "Unable to connect to %s:%s\n", host, service);
	return -1;
    }

    return sock;
}

int main(int argc, char **argv){
    int sock, bytes, write_bytes=0;

    sock = connect_inet(argv[1], argv[2]);
    if (sock < 0) exit(EXIT_FAILURE);

    printf("Welcome to server tester!\n");

    printf("Enter a message to send to server: \n");
    printf("NOTE: Any message < 7 bytes is flagged as incomplete by the server;\n\tthe server won't respond until it recieves at least 7 bytes!\n");
    char msg[BUFSIZE+1];
    while ( (bytes = read(STDIN_FILENO, msg, BUFSIZE)) > 0 )
    {
	// send test message to server
	write_bytes += write(sock, msg, bytes-1);

	if (write_bytes == -1){
	    perror("write");
	    close(sock);
	    return EXIT_FAILURE;
	}

	printf("wrote %d bytes\n", write_bytes);

	if ( write_bytes >= 7 ) {
	    write_bytes = 0;
	    int read_bytes;
	    if ( (read_bytes = read(sock, msg, BUFSIZE)) > 0 ) {
		msg[read_bytes] = '\0';
		printf("Server replied with: \n%s\n", msg);
	    }
	    else {
		printf("Didn't recieve anything back!\n");
	    }
	}

	printf("\nEnter a message to send to server: \n");

    }

    printf("Closing server tester!\n");
    close(sock);
    return EXIT_SUCCESS;
}

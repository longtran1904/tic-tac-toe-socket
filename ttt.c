#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdbool.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include"protocol.h"
#define BUFLEN 256

// socket/network declaration functions
int connect_inet(char *host, char *service);

bool isDone(char game[9]){
    int i;
    for (i = 0; i < 9; i++)
        if (game[i] == ' ') return false;
    
    return true;
}

void print_board(char game[9]){
    int i,j, count = 0;
    for (i = 0; i < 7; i++){
        for (j = 0; j < 7; j++)
            if (i % 2 == 0) printf("-");
            else {
                if (j % 2 == 0) printf("|");
                else {
                    if (game[count] != ' ')
                        printf("%c", game[count++]);
                    else printf("%d", ++count);
                }
            }    
        printf("\n");
    }
}

int main(int argc, char **argv){
    char game[9];
    int i;
    int sock, bytes, write_bytes;

    sock = connect_inet(argv[1], argv[2]);
    if (sock < 0) exit(EXIT_FAILURE);

    printf("Welcome to Tic-Tac-Toe!!!\n");  
    printf("Enter your username:\n");
    char user[BUFLEN];
    if ((bytes = read(STDIN_FILENO, user, BUFLEN)) > 0){

        if (bytes == -1){
            perror("read username");
            return EXIT_FAILURE;
        }

        // generate PLAY message to server
        char* msg_buffer = malloc(sizeof(char));
        int buf_len = 1;
        int play_len = play(&msg_buffer, buf_len, user, bytes - 1); // exclude \n from input

        // send PLAY message to server
        write_bytes = write(sock, msg_buffer, play_len);

        // free
        free(msg_buffer);

        if (write_bytes == -1){
            perror("write");
            close(sock);

            return EXIT_FAILURE;
        }
        printf("write %d bytes\n", write_bytes);
    }

    // TODO: Retrieve game[9] from server
    for (i = 0; i < 9; i++)
        game[i] = ' ';
    
    char buf;
    bool validMove = true;

    do {
        if (!validMove) {
            printf("Wrong format! Type a number from 1 to 9\n");
            validMove = true;
        }
        print_board(game);
        printf("Please choose cell number to make a move (1 to 9)\n");
        if (read(STDIN_FILENO, &buf, 1) > 0){
            if (buf - '0' < 0 || buf - '0' > 9){
                validMove = false;
                continue;
            }
            // TODO: send move signal to server
            game[(buf - '0') - 1] = 'X';
        }
    } while (!isDone(game));
    return EXIT_SUCCESS;
}

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
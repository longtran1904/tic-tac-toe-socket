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

void print_board(char game[9]){
    printf("The current board is: \n");
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

message *prev_msg_in;
bool just_made_move = false;
char latest_board[9];

void send_move_rsgn_or_draw( int sock, char *board ) {
    printf("Would you like to:\n\t[0] - Make a move\n\t[1] - Resign\n\t[2] - Request a draw\n");

    char *write_buf = malloc(sizeof(char));
    int buf_len = 1;

    char buf[3];
    read(STDIN_FILENO, &buf, 3);

    if ( buf[0] == '0' ) {
	print_board( board );
	printf("Please choose a coordinate ( ex: '1,2' '3,3' '2,1' ):\n");
	read(STDIN_FILENO, &buf, 3);
	pair pos = { (int) buf[0] - '0', (int) buf[2] - '0' };
	int move_len = move(&write_buf, buf_len, 'X', pos);
	send_message( sock, write_buf, move_len );
	just_made_move = true;
	free(write_buf);
    }
    else if ( buf[0] == '1' ) {
	int rsgn_len = resign(&write_buf, buf_len);
	send_message( sock, write_buf, rsgn_len );
	free(write_buf);
    }
    else {
	int draw_len = draw(&write_buf, buf_len, SUGGEST);
	send_message( sock, write_buf, draw_len);
	free(write_buf);
    }
}

int message_responder( int sock, message *msg_in ) {
    char msg_code[5];
    strcpy(msg_code, msg_in->code);

    if ( strcmp(msg_code, "WAIT") == 0 ) {
	return 0;
    }
    else if ( strcmp(msg_code, "BEGN") == 0 ) {
	if ( msg_in->role == 'X' ) {
	    char board[9] = {'.','.','.','.','.','.','.','.','.'};
	    send_move_rsgn_or_draw( sock, board );
	    return 0;
	} // if not assigned to 'X', we must wait
	return 0;
    }
    else if ( strcmp(msg_code, "MOVD") == 0 ) {
	printf("made it here!\n");
	if ( just_made_move ) {
	    memcpy(latest_board, msg_in->board, 9);
	    just_made_move = false;
	    return 0;
	} // our turn
	else {
	    memcpy(latest_board, msg_in->board, 9);
	    send_move_rsgn_or_draw( sock, msg_in->board );
	    return 0;
	}
    }
    else if ( strcmp(msg_code, "INVL") == 0 ) {
	//message_responder( sock, prev_msg_in );
	return 0;
    }
    else if ( strcmp(msg_code, "DRAW") == 0 ) {
	if ( msg_in->msg == 'S' ) {
	    print_board( msg_in->board );
	    printf("Your opponent has suggested a draw!\n\
		    \tif you agree: [y]\n\tif you disagree: [n]\n");
	    char buf;
	    read(STDIN_FILENO, &buf, 1);
	    char *write_buf = malloc(sizeof(char));
	    int buf_len = 1;
	    if ( buf == 'y' ) {
		int draw_len = draw(&write_buf, buf_len, ACCEPT);
		send_message( sock, write_buf, draw_len );
		free(write_buf);
		return 0;
	    } // you rejected the draw
	    else {
		int draw_len = draw(&write_buf, buf_len, REJECT);
		send_message( sock, write_buf, draw_len );
		free(write_buf);
		return 0;
	    }
	} // msg_in->msg = 'R'
	else {
	    printf("Your request for a draw was rejected!\n");
	    send_move_rsgn_or_draw( sock, latest_board );
	    return 0;
	}
    }  // msg_code = "OVER"
    else {
	if ( msg_in->outcome == 'W' ) {
	    printf("GAME OVER! You've won!\n");
	}
	else if ( msg_in->outcome == 'L' ) {
	    printf("GAME OVER! You've lost!\n");
	} // draw
	else {
	    printf("GAME OVER! It was a draw!\n");
	}
	return -1; // terminate
    }
}

void read_data( int sock ){
    char buf[BUFSIZE+1];
    int bytes=0, msg_size=0, buf_offset=0;

    while ((bytes += read(sock, buf+buf_offset, BUFSIZE-buf_offset))) {
	char *msg_str = grab_msg_shift_buf( buf, bytes, &msg_size, &buf_offset );
	if ( msg_str == NULL ) {
	    if ( msg_info == INCOMPLETE_MSG ) {
		continue;
	    } // corrupted message: break and close socket
	    else {
		perror("grab_msg_shift_buf() error");
		break;
	    }
	} // successfully grabbed message
	buf[bytes] = '\0';

	message *msg_struct = parse_msg( msg_str, msg_size );
	if ( msg_struct == NULL ) {
	    free(msg_str);
	    perror("parse_msg() error");
	    break; // close connection
	}
	// successfully parsed message into struct
	// respond to message
	printf("msg_code: %s\n", msg_struct->code);
	int terminate = message_responder( sock, msg_struct );
	//memcpy(prev_msg_in, msg_struct, sizeof(message));
	free(msg_str);
	free(msg_struct);
	if ( terminate ) break;
	if ( buf_offset == 0 ) bytes = 0;
    }

    printf("Terminating connection!\n");
    close(sock);

}

int main(int argc, char **argv){
    int sock, bytes, write_bytes;

    sock = connect_inet(argv[1], argv[2]);
    if (sock < 0) exit(EXIT_FAILURE);

    printf("Welcome to Tic-Tac-Toe!!!\n");  
    printf("Enter your username:\n");
    char user[BUFSIZE];
    if ((bytes = read(STDIN_FILENO, user, BUFSIZE)) > 0){
	// generate PLAY message 
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
    } // failed to read username
    else {
	perror("read username");
	return EXIT_FAILURE;
    }

    read_data( sock );

    return EXIT_SUCCESS;

}

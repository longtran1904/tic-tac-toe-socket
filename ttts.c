// NOTE: must use option -pthread when compiling!
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include "game_database.h"
#include "game_logic.h"
#include "socket_buf_mem.h"
#include "protocol.h"

#define QUEUE_SIZE 8
#define BUFSIZE 265
#define PORTSIZE 10
#define HOSTSIZE 100
#define MAX_NUM_OF_GAMES 50

#ifndef DEBUG
    #define DEBUG 0
#endif

int is_active[13000] = { 0 };

volatile int active = 1;

void handler(int signum){
	active = 0;
}

// set up signal handlers for primary thread
// return a mask blocking those signals for worker threads
// FIXME should check whether any of these actually succeeded
void install_handlers(sigset_t *mask)
{
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
    
    sigemptyset(mask);
    sigaddset(mask, SIGINT);
    sigaddset(mask, SIGTERM);
}

// data to be sent to worker threads
struct connection_data {
	struct sockaddr_storage addr;
	socklen_t addr_len;
	int fd;
};

int open_listener(char *service, int queue_size) {
    struct addrinfo hint, *info_list, *info;
    int error, sock;

    // initialize hints
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family   = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags    = AI_PASSIVE;

    // obtain information for listening socket
    error = getaddrinfo(NULL, service, &hint, &info_list);
    if (error) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
        return -1;
    }

    // attempt to create socket
    for (info = info_list; info != NULL; info = info->ai_next) {
        sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

        // if we could not create the socket, try the next method
        if (sock == -1) continue;

        // bind socket to requested port
        error = bind(sock, info->ai_addr, info->ai_addrlen);
        if (error) {
            close(sock);
            continue;
        }

        // enable listening for incoming connection requests
        error = listen(sock, queue_size);
        if (error) {
            close(sock);
            continue;
        }

        // if we got this far, we have opened the socket
        break;
    }

    freeaddrinfo(info_list);

    // info will be NULL if no method succeeded
    if (info == NULL) {
        fprintf(stderr, "Could not bind\n");
        return -1;
    }

    return sock;
}




game_node *waiting_game = NULL;

int send_invld( int sock ) {
    char *msg_buf = malloc(sizeof(char));
    int buf_len = 1, invalid_len;

    switch (msg_info) {
	case BAD_NAME_FLD: 
	    // generate INVLD message 
	    invalid_len = invalid(&msg_buf, buf_len,\
		    "Invalid name: cannot exceed 100 chars or contain a bar!", 55);
	    break;
	case BAD_ROLE_FLD:
	    invalid_len = invalid(&msg_buf, buf_len,\
		    "Invalid role: must be 'X' or 'O'!", 33);
	    break;
	case BAD_POSITION_FLD:
	    invalid_len = invalid(&msg_buf, buf_len,\
		    "Invalid position: must be (1/2/3, 1/2/3)!", 41);
	    break;
	case BAD_BOARD_FLD:
	    invalid_len = invalid(&msg_buf, buf_len,\
		    "Invalid board: must contain only 'X', 'O', or '.'!", 50);
	    break;
	case BAD_MSG_FLD:
	    invalid_len = invalid(&msg_buf, buf_len,\
		    "Invalid message: must be 'S', 'A', 'R'!", 39);
	    break;
	case UNEXPECTED:
	    invalid_len = invalid(&msg_buf, buf_len,\
		    "Message not expected at this time!", 34);
	    break;
	default:
	    invalid_len = invalid(&msg_buf, buf_len,\
		    "Garbage / corrupt message recieved: terminating connection!", 59);
	    send_message( sock, msg_buf, invalid_len );
	    free(msg_buf);
	    return -1; // signifies we should terminate
    }

    // send INVLD message to client
    int send_stat = send_message( sock, msg_buf, invalid_len );

    // free
    free(msg_buf);

    return send_stat;
}

int update_state_send_msg( int sock, message *msg_in, game_node *game ) {
    char *msg_buf = malloc(sizeof(char));
    int buf_len = 1, send_stat;

    char *msg_code = msg_in->code;

    // server_state is 0
    if ( game->server_state == 0 ) {
	// if we're in server_state 0, msg_code must be "PLAY"

	int wait_len = wait_game(&msg_buf, buf_len);

	// game was just initiated with this one player
	if ( sock == game->sock1 ) {
	    // send WAIT to player1
	    strcpy(game->sock1_name, msg_in->name);
	    game->sock1_name_len = msg_in->name_len;
	    send_stat = send_message( sock, msg_buf, wait_len );
	    free(msg_buf);
	    if ( send_stat ) return -1;
	    waiting_game = game;
	    return 0;
	} // game has a second player!
	else {
	    game->sock2 = sock;
	    strcpy(game->sock2_name, msg_in->name);
	    game->sock2_name_len = msg_in->name_len;
	    // send WAIT to player2
	    send_stat = send_message( game->sock2, msg_buf, wait_len );
	    if ( send_stat ) {
		free(msg_buf);
		return -1;
	    }

	    srand(time(NULL));
	    int rand_num = rand() > RAND_MAX / 2;
	    //if (DEBUG) printf("rand_num: %d\n", rand_num ); // either 0 or 1
	    // determine who goes first / gets role 'X'
	    if ( rand_num == 1 ) {
		game->sock1_role = 'X';
		game->sock2_role = 'O';
		game->up_next = game->sock1;
	    }
	    else {
		game->sock1_role = 'O';
		game->sock2_role = 'X';
		game->up_next = game->sock2;
	    }

	    // send BEGN to player1
	    printf("Initiating game between \'%s\' and \'%s\'!\n", game->sock1_name, game->sock2_name);
	    int begn_len = begin(&msg_buf, buf_len, game->sock1_role, 
		    game->sock2_name, game->sock2_name_len);
	    send_stat = send_message( game->sock1, msg_buf, begn_len );
	    if ( send_stat ) {
		free(msg_buf);
		return -1;
	    }
	    // send BEGN to player2
	    begn_len = begin(&msg_buf, buf_len, game->sock2_role, 
		    game->sock1_name, game->sock1_name_len);
	    send_stat = send_message( game->sock2, msg_buf, begn_len );
	    free(msg_buf);
	    if ( send_stat ) return -1;

	    // update server_state
	    game->server_state = 1;

	    waiting_game = NULL;

	    return 0; // success
	}
    } // server_state is 1
    else if ( game->server_state == 1 ) {
	if ( strcmp(msg_code, "MOVE") == 0 ) {
	    // verify it is the players turn and they're using correct role
	    if ( game->up_next == sock ) {
		if ( ((sock == game->sock1) && (msg_in->role != game->sock1_role)) ||
		    ((sock == game->sock2) && (msg_in->role != game->sock2_role)) ) {
		    // role doesn't match: player using incorrect role!
		    free(msg_buf);
		    msg_info = BAD_ROLE_FLD;
		    return send_invld( sock );
		} // player is using correct role and it's their turn
		int board_index = 3*(msg_in->position.x-1) + (msg_in->position.y-1);
		if ( game->board[board_index] == '.' ) {
		    game->board[board_index] = msg_in->role;
		    enum win_state game_stat = isDone( game->board );

		    if ( game_stat == UNFINISHED ) {
			game->up_next = 
			    ( sock == game->sock1) ? game->sock2 : game->sock1;
			int moved_len =
			    move_board(&msg_buf, buf_len, msg_in->role,
				    msg_in->position, game->board);

			if (DEBUG) {
			    char* printable_buf = malloc(sizeof(char) * (moved_len+1));
			    memcpy(printable_buf, msg_buf, moved_len);
			    printable_buf[moved_len] = '\0';
			    printf("message length %d | sending message |%s\n", moved_len, printable_buf);
			    printf("Game Board:\n");
			    for (int i = 0; i < 9; i++) printf("%c", game->board[i]);
			    printf("\n");
			    free(printable_buf);
			}

			send_stat = send_message( game->sock1, msg_buf, moved_len);
			if ( send_stat ) {
			    free(msg_buf);
			    return -1;
			}
			if (DEBUG) printf("sent MOVD to sock1 %d!\n", game->sock1);

			send_stat = send_message( game->sock2, msg_buf, moved_len );
			if (DEBUG) printf("sent MOVD to sock2 %d!\n", game->sock2);
			free(msg_buf);
			if ( send_stat ) return -1;

			return 0; // success
		    }
		    else if ( game_stat == X_WIN || game_stat == O_WIN ) {
			char *reason;
			if ( game_stat == X_WIN ) {
			    reason = "'X' has completed a line!";
			} // O won
			else {
			    reason = "'O' has completed a line!";
			}
			int over_len = over(&msg_buf, buf_len, WIN, reason, 30);   
			send_stat = send_message( sock, msg_buf, over_len );
			if ( send_stat ) {
			    free(msg_buf);
			    return -1;
			}
			over_len = over(&msg_buf, buf_len, LOSE, reason, 30);   
			int sock_loser = (sock == game->sock1) ? game->sock2 : game->sock1;
			send_stat = send_message( sock_loser, msg_buf, over_len );
			free(msg_buf);
			if ( send_stat ) return -1;
			printf("Terminating game between \'%s\' and \'%s\'!\n", game->sock1_name, game->sock2_name);
			is_active[game->sock1] = 0;
			is_active[game->sock2] = 0;
			return remove_existing_game( sock );

			//
			// !!!!!!!!!!!!!!!
			//
			// 
			//TODO: signal both threads to terminate
			//
			// !!!!!!!!!!!!!!
			// 
		    } // it's a DRAW
		    else {
			char *reason = "The grid is full: it's a draw!";
			int over_len = over(&msg_buf, buf_len, DRAW, reason, 30);   
			send_stat = send_message( game->sock1, msg_buf, over_len );
			if ( send_stat ) {
			    free(msg_buf);
			    return -1;
			}
			send_stat = send_message( game->sock2, msg_buf, over_len );
			free(msg_buf);
			if ( send_stat ) return -1;
			printf("Terminating game between \'%s\' and \'%s\'!\n", game->sock1_name, game->sock2_name);
			is_active[game->sock1] = 0;
			is_active[game->sock2] = 0;
			return remove_existing_game( sock );
		    }
		} // position already taken
		else {
		    msg_info = BAD_POSITION_FLD;
		    free(msg_buf);
		    return send_invld( sock );
		}
	    } // player is NOT up next
	    else {
		msg_info = UNEXPECTED;
		free(msg_buf);
		return send_invld( sock );
	    }
	}
	else if ( strcmp(msg_code, "RSGN") == 0 ) {
	    char *reason = "A player has resigned!";
	    int over_len = over(&msg_buf, buf_len, DRAW, reason, 22);
	    send_stat = send_message( game->sock1, msg_buf, over_len );
	    if ( send_stat ) {
		free(msg_buf);
		return -1;
	    }
	    send_stat = send_message( game->sock2, msg_buf, over_len );
	    free(msg_buf);
	    if ( send_stat ) return -1;
	    printf("Terminating game between \'%s\' and \'%s\'!\n", game->sock1_name, game->sock2_name);
	    is_active[game->sock1] = 0;
	    is_active[game->sock2] = 0;
	    return remove_existing_game( sock );
	} // msg_code is "DRAW" and should be 'S' for suggest because we are in state 1
	else {
	    int draw_len = draw(&msg_buf, buf_len, SUGGEST);
	    int sock_oppnt = (sock == game->sock1) ? game->sock2 : game->sock1;
	    send_stat = send_message( sock_oppnt, msg_buf, draw_len );
	    free(msg_buf);
	    if ( send_stat ) return -1;
	    game->server_state = 2;
	    return 0; // success
	}

    } // server_state is 2: msg_code == "DRAW"
    else {
	if ( msg_in->msg == 'A' ) {
	    char *reason = "Agreed-upon draw!";
	    int over_len = over(&msg_buf, buf_len, DRAW, reason, 17);
	    send_stat = send_message( game->sock1, msg_buf, over_len );
	    if ( send_stat ) {
		free(msg_buf);
		return -1;
	    }
	    send_stat = send_message( game->sock2, msg_buf, over_len );
	    free(msg_buf);
	    if ( send_stat ) return -1;
	    printf("Terminating game between \'%s\' and \'%s\'!\n", game->sock1_name, game->sock2_name);
	    is_active[game->sock1] = 0;
	    is_active[game->sock2] = 0;
	    return remove_existing_game( sock );
	} 
	else if ( msg_in->msg == 'R' ) {
	    int draw_len = draw(&msg_buf, buf_len, REJECT);
	    int sock_oppnt = (sock == game->sock1) ? game->sock2 : game->sock1;
	    send_stat = send_message( sock_oppnt, msg_buf, draw_len );
	    free(msg_buf);
	    if ( send_stat ) return -1;
	    game->server_state = 1;
	    return 0; // success
	} // error: should not receive 'S' in this state
	else {
	    msg_info = UNEXPECTED;
	    free(msg_buf);
	    return send_invld( sock );
	}
    }
}


char *server_state[3][4] = {
    { "PLAY", NULL },
    { "MOVE", "DRAW", "RSGN", NULL },
    { "DRAW", NULL },
};

// assumes msg_in is NOT NULL
int message_responder( int sock, message *msg_in ) {
    game_node *game = grab_game( sock );

    if ( game == NULL ) { // we have a new player!
	if ( waiting_game != NULL ) {
	    game = waiting_game;
	} // there is no game with one player waiting: start a new one
	else {
	    game = create_new_game( sock );
	}
    }

    char **lst_of_valid_next_codes = server_state[game->server_state];

    // confirm the message is expected
    int i = 0;
    char *vld_nxt_code = lst_of_valid_next_codes[i];
    while ( vld_nxt_code != NULL ) {
	if ( strcmp(msg_in->code, vld_nxt_code) == 0 ) { break; }
	vld_nxt_code = lst_of_valid_next_codes[++i];
    }

    // the message was not expected
    if ( vld_nxt_code == NULL ) {
	if (DEBUG) printf("message unexpected!\n");
	msg_info = UNEXPECTED;
	return send_invld( sock );
    }
    // the message is expected
    return update_state_send_msg( sock, msg_in, game );
}

void *read_data( void *arg ){
    struct connection_data *con = arg; 
    char host[HOSTSIZE], port[PORTSIZE];
    int error, msg_size=0;

    // load buf info for given socket 'con->fd'
    sock_buf_node *curr = get_buf_info( con->fd );
    if ( curr == NULL ) {
	curr = create_new_node( con->fd );
    }

    error = getnameinfo(
	    (struct sockaddr *)&con->addr, con->addr_len,
	    host, HOSTSIZE,
	    port, PORTSIZE,
	    NI_NUMERICSERV
	    );
    if (error) {
	fprintf(stderr, "getnameinfo: %s\n", gai_strerror(error));
	strcpy(host, "??");
	strcpy(port, "??");
    }

    printf("Connection from %s:%s\n", host, port);

    while (active /*&& is_active[con->fd] */  && 
	    (curr->bytes += read(con->fd, curr->buf+curr->buf_offset, BUFSIZE-curr->buf_offset))) {

	//curr->buf[curr->bytes] = '\0';
	//printf("[%s:%s] read %d bytes %s\n", host, port,
	//	curr->bytes, curr->buf);

	char *msg_str;
	int respond_stat = 0;
	int terminate = 0;
	// in case we recieved 2 or more complete messages
	while ( (msg_str = grab_msg_shift_buf( curr->buf, curr->bytes, 
		&msg_size, &curr->buf_offset )) )
	{
	    // successfully grabbed message
	    message *msg_struct = parse_msg( msg_str, msg_size );
	    if ( msg_struct == NULL ) {
		free(msg_str);
		terminate = send_invld( con->fd );
		if ( terminate ) { // close connection
		    printf("terminating because send_invld\n");
		    perror("parse_msg() error");
		    break;
		} // try to read again... !!!NOTE: this overrides buf if had > 1 msg
		curr->bytes = 0;
		curr->buf_offset = 0;
		break;
	    }
	    // successfully parsed message into struct
	    // respond to message
	    respond_stat = message_responder( con->fd, msg_struct );
	    free(msg_str);
	    free(msg_struct);
	    if ( respond_stat ) break;
	    curr->bytes = curr->buf_offset;
	    msg_info = ALL_GOOD;
	    if ( curr->buf_offset == 0 ) {
		curr->bytes = 0;
		break;
	    }

	    //curr->buf[curr->bytes] = '\0';
	    //printf("bytes: %d\n buf_offset: %d\n buf: %s\n", curr->bytes, curr->buf_offset, curr->buf);
	} 

	// no more complete messages in buf
	if ( terminate || respond_stat ) break;

	if ( msg_info != BAD_FORMAT ) {
	    if ( msg_info == INCOMPLETE_MSG ) {
		continue;
	    } // msg_info not INCOMPLETE_MSG but some other prob: reset read
	    else {
		curr->bytes = 0;
		curr->buf_offset = 0;
		continue;
	    }
	} // corrupted message: break and close socket
	else {
	    perror("INVALID MESSAGE FORMAT RECEIVED");
	    char *msg_buf = malloc(BUFSIZE);
	    int buf_len = 1;
	    int invalid_len = invalid(&msg_buf, buf_len,\
		    "Garbage / corrupt message recieved: terminating connection!", 59);
	    send_message( con->fd, msg_buf, invalid_len );
	    free(msg_buf);
	    printf("termination reason: %s\n", get_msg_info_str(msg_info));
	    break;
	}
    }

    if (curr->bytes == 0){
	printf("[%s:%s] got EOF\n", host, port);
    } else if (curr->bytes == -1){
	printf("[%s:%s] terminating: %s\n", host, port, strerror(error));
    } else{
	printf("[%s:%s] terminating\n", host, port);
    }

    printf("closing socket: %d\n", con->fd);
    close(con->fd);

    rm_sock_buf_node( con->fd );

    free(con);

    return NULL;
}


int main(int argc, char ** argv){
    sigset_t mask;
    struct connection_data *con;
    int error;
    pthread_t tid;

    char *service = argc == 2 ? argv[1] : "15000";

    install_handlers(&mask);

    int listener = open_listener(service, QUEUE_SIZE);
    if (listener < 0) exit(EXIT_FAILURE);

    printf("Listening for incoming connections on %s\n", service);

    while (active) {
	con = (struct connection_data *)malloc(sizeof(struct connection_data));
	con->addr_len = sizeof(struct sockaddr_storage);

	con->fd = accept(listener, 
		(struct sockaddr *)&con->addr,
		&con->addr_len);

	if (con->fd < 0) {
	    perror("accept");
	    free(con);
	    // TODO check for specific error conditions
	    continue;
	}

	// temporarily disable signals
	// (the worker thread will inherit this mask, ensuring that SIGINT is
	// only delivered to this thread)
	error = pthread_sigmask(SIG_BLOCK, &mask, NULL);
	if (error != 0) {
	    fprintf(stderr, "sigmask: %s\n", strerror(error));
	    exit(EXIT_FAILURE);
	}

	is_active[con->fd] = 1;

	error = pthread_create(&tid, NULL, read_data, con);
	if (error != 0) {
	    fprintf(stderr, "pthread_create: %s\n", strerror(error));
	    close(con->fd);
	    free(con);
	    continue;
	}

	// automatically clean up child threads once they terminate
	pthread_detach(tid);

	// unblock handled signals
	error = pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
	if (error != 0) {
	    fprintf(stderr, "sigmask: %s\n", strerror(error));
	    exit(EXIT_FAILURE);
	}
    }

    puts("Shutting down");
    close(listener);

    // returning from main() (or calling exit()) immediately terminates all
    // remaining threads

    // to allow threads to run to completion, we can terminate the primary thread
    // without calling exit() or returning from main:
    //   pthread_exit(NULL);
    // child threads will terminate once they check the value of active, but
    // there is a risk that read() will block indefinitely, preventing the
    // thread (and process) from terminating

    // to get a timely shut-down of all threads, including those blocked by
    // read(), we will could maintain a global list of all active thread IDs
    // and use pthread_cancel() or pthread_kill() to wake each one

    return EXIT_SUCCESS;
}

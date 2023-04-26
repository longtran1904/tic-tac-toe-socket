# tic-tac-toe-socket
CS 214: Systems Programming
Professor Menendez
Julian Herman (netID: jbh113)
Long Tran (netID: lht21)

A tic-tac-toe online server allows multiple concurrent games with multi-threading
Implemented in C using standard libraries: socket, pthread, pthread_mutex.

## Makefile Instructionis
- 'make client' (default: 'make') runs tic-tac-toe client
- 'make server' runs tic-tac-toe server
- 'make protocol_tester' runs test_protocol.c -> see original messages generated && received by protocol.c
- 'make server_tester' runs test_server -> program to interact with server without tic-tac-toe UI; interact by using protocol messages

## Tic-tac-toe Server
### Protocol
  - Messages sent by Server (*) 
    - WAIT
    - DRAW
    - MOVD
    - INVL
    - OVER
  - Messages that Server recognize
    - PLAY
    - MOVE
    - RSGN
    - DRAW
### Server designing decision
  - Each user connection to server will create its own thread on server
  - Data Structures:
    - Linked List: for controlling states of games

          typedef struct game_node {
          int sock1;
          char sock1_name[101];
          int sock1_name_len;
          char sock1_role;
          int sock2;
          char sock2_name[101];
          int sock2_name_len;
          char sock2_role;
          int up_next; // value of sock1 or sock2 
          int server_state; // 0, 1, or 2
          char board[9];
          pthread_mutex_t lock;
          struct game_node *next_game;
          } game_node;

    - Linked List: for controlling buffers for each socket/user

          typedef struct sock_buf_node {
          int sock;
          char *buf;
          int bytes;
          int buf_offset;
          struct sock_buf_node *next;
          } sock_buf_node;

    - Use of locks for multi-threading:
      - Every response to a message from client will block the game_node the client is using.
      - After a user is done with responding messages back to server, the lock is free on that game_node

  - Other designs:
    - A matched client (client is in game with another client) terminates his program will signal the server & the other client to terminate the connection and clean that game_node

### Testing Plan
  - Purpose: preventing unexpected messages send to server for unexpected behaviors
#### Unit/Messages Testing
  - Implemented test_protocol.c for testing messages generated and message buffer grabber.

#### Single Game Testing
  - In matching phase:
    - Before client receives BEGN message
    - Messages allowed: PLAY
    - if client send any messages beside PLAY, server detects that as non valid message at this phase.
      ***Input***: MOVE|6|X|2,2|
      ***Expected***: INVL|{msg size}|Message not expected at this time!|

  - After matching phase:
    - After client receives BEGN message
    - Messages allowed: MOVE, RSGN, DRAW
    - if client send PLAY:
      ***Input***: PLAY|{msg size}|{name}|
      ***Expected***: INVL|{msg size}|Message not expected at this time!|

  - If a client send a message when it's not his turn, server will send back: `INVL|35|Message not expected at this time!|`

  - Recognize wrong role in MOVE message:
    - If a client send MOVE message with role not assigned from server, server will reject that message

  - Prevent hacking situations (do not allow client to use messages supposed to be used by server)
  - Any message in (*) will be detected as error message and terminate connection

  - Make sure server states reset when:
    - Terminating a game
    - Creating a new game
  - Consecutively match and terminate 10 games
    - Expected every game to run normally

#### Concurrent Game Testing
  - Play 3 concurrent games at a time without crashing the server.
  - ***Expecting***: 3 games play as normal.

  - Playing 3 concurrent games. Have 1 client to terminate program
  - ***Expecting***: only game that client plays will be terminated
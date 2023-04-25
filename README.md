# tic-tac-toe-socket
CS 214: Systems Programming
Professor Menendez
Julian Herman (netID: jbh113)
Long Tran (netID: lht21)

A tic-tac-toe online server allows multiple concurrent games 
Implemented in C using standard libraries: socket, pthread.

## Makefile Instructions
- 'make' (default) runs tic-tac-toe client
- 'make server' runs tic-tac-toe server

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
### Testing Plan
  - Purpose: preventing unexpected messages send to server for unexpected behaviors
#### Unit/Messages Testing
  - Implemented test_protocol.c for testing messages generated and message buffer grabber.

#### Single Game Testing
  - In matching phase:
    - Before client receives BEGN message
    - Messages allowed: PLAY
    - if client send any messages beside PLAY, server detects that as non valid message at this phase.
    `MOVE|6|X|2,2|
    Expected: INVL|{msg size}|Message not expected at this time!|`

  - After matching phase:
    - After client receives BEGN message
    - Messages allowed: MOVE, RSGN, DRAW
    - if client send PLAY:
    `PLAY|{msg size}|{name}|
    Expected: INVL|{msg size}|Message not expected at this time!|`

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
  - Expecting: 3 games play as normal.
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

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

// initialize game list's head pointer, tail pointer and mutex lock
void game_list_init(void);

// returns pointer to new game with sock1=sock
game_node *create_new_game( int sock );

// finds, removes, and frees existing game data associated with sock
// returns 0 on success
// returns -1 on failure (game not found)
int remove_existing_game( int sock );

// returns the game_node of game associated with sock
// if not found, returns NULL
// this function locks the game_node returned
// assure to set_game() so that other threads can access.
game_node *grab_game( int sock );

// this function takes game_node and free the lock
void set_game( game_node *game );

// checks if name is found in any of the current games 
// if found, returns false, else true 
bool name_is_unique( char *name );

// return a game which has 1 player waiting for a game to start
game_node *isWaiting();


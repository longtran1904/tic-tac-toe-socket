#include <stdlib.h>
#include <stdbool.h>

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
    struct game_node *next_game;
} game_node;

// returns pointer to new game with sock1=sock
game_node *create_new_game( int sock );

// finds, removes, and frees existing game data associated with sock
// returns 0 on success
// returns -1 on failure (game not found)
int remove_existing_game( int sock );

// returns the game_node of game associated with sock
// if not found, returns NULL
game_node *grab_game( int sock );

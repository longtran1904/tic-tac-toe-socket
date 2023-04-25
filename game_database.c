#include "game_database.h"
#include <string.h>

game_node *list_head = NULL;
game_node *list_tail = NULL;

// returns pointer to new game with sock1=sock
game_node *create_new_game( int sock ) {
    // create game_node
    game_node *new_game = malloc( sizeof(game_node) );
    new_game->sock1 = sock;
    new_game->sock2 = -1;
    new_game->server_state = 0;
    memcpy(new_game->board, ".........", 9);
    new_game->next_game = NULL;

    // update list_tail
    if ( list_tail != NULL ) list_tail->next_game = new_game;
    list_tail = new_game;

    // update list_head
    if ( list_head == NULL ) list_head = new_game;

    return new_game;
}

// finds, removes, and frees existing game data associated with sock
// returns 0 on success
// returns -1 on failure (game not found)
int remove_existing_game( int sock ) {
    game_node *prev_game = list_head;
    game_node *tmp = list_head;
    while( tmp != NULL ) {
	if ( (tmp->sock1 == sock) || (tmp->sock2 == sock) ) {
	    // found the game to be removed
	    if ( list_head == list_tail ) {
		list_head = NULL;
		list_tail = NULL;
	    }
	    else if ( tmp == list_head ) {
		list_head = tmp->next_game;
	    }
	    else if ( tmp == list_tail ) {
		list_tail = prev_game;
		list_tail->next_game = NULL;
	    } // tmp is not list_head or list_tail
	    else {
		prev_game->next_game = tmp->next_game;
	    }
	    free(tmp);
	    return 0; // success
	}
	prev_game = tmp;
	tmp = tmp->next_game;
    }
    return -1; // failure 
}

// returns the game_node of game associated with sock
// if not found, returns NULL
game_node *grab_game( int sock ) {
    game_node *tmp = list_head;
    while( tmp != NULL ) {
	if ( (tmp->sock1 == sock) || (tmp->sock2 == sock) ) { return tmp; }
	tmp = tmp->next_game;
    }
    return NULL; // there is no game associated with sock
}

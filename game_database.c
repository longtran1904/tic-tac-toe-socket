#include "game_database.h"

game_node *list_head;
game_node *list_tail;
pthread_mutex_t list_lock;

void game_list_init(void){	
	list_head = NULL;
	list_tail = NULL;
	pthread_mutex_init(&list_lock, NULL);
}

// returns pointer to new game with sock1=sock
game_node *create_new_game( int sock ) {

	pthread_mutex_lock(&list_lock);
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

	pthread_mutex_unlock(&list_lock);
    return new_game;
}

// finds, removes, and frees existing game data associated with sock
// returns 0 on success
// returns -1 on failure (game not found)
int remove_existing_game( int sock ) {

	pthread_mutex_lock(&list_lock);
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
		pthread_mutex_unlock(&list_lock);
	    return 0; // success
	}
	prev_game = tmp;
	tmp = tmp->next_game;
    }

	pthread_mutex_unlock(&list_lock);
    return -1; // failure 
}

// returns the game_node of game associated with sock
// if not found, returns NULL
game_node *grab_game( int sock ) {
    game_node *tmp = list_head;
    while( tmp != NULL ) {
	if ( (tmp->sock1 == sock) || (tmp->sock2 == sock) ) { 
		pthread_mutex_lock(&tmp->lock); // lock this game
		return tmp; 
	}
	tmp = tmp->next_game;
    }
    return NULL; // there is no game associated with sock
}

void set_game( game_node *game ){
	pthread_mutex_unlock(&game->lock);
}

// checks if name is found in any of the current games 
// if found, returns false, else true 
bool name_is_unique( char *name ) {
	pthread_mutex_lock(&list_lock);
    game_node *tmp = list_head;
    while( tmp != NULL ) {
	if ( strcmp(tmp->sock1_name, name)==0 ||
		strcmp(tmp->sock2_name, name)==0 ) { 
			pthread_mutex_unlock(&list_lock);
			return false; 
	}
	tmp = tmp->next_game;
    }
	pthread_mutex_unlock(&list_lock);
    return true; // name is unique 
}

// return a game_node pointer if there's a waiting game
// otherwise, NULL pointer
game_node *isWaiting(){
	pthread_mutex_lock(&list_lock);
	if (list_tail == NULL || list_tail->sock2 != -1) 
	{
		pthread_mutex_unlock(&list_lock);
		return NULL;
	}
	pthread_mutex_unlock(&list_lock);
	return list_tail;
}

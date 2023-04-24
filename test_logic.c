#include"game_logic.h"
#include<assert.h>
#include<stdio.h>

int main(){
    // diagonal
    char *board;
    board = "X...X...X";
    enum win_state win = isDone(board); 
    printf("win main diagonal: %d\n", win);
    assert( win == X_WIN );

    board = "..X.X.X..";
    win = isDone(board);
    printf("win sub diagonal: %d\n", win);
    assert( win == X_WIN);

    board = "O...O...O";
    win = isDone(board);
    printf("win main diagonal O: %d\n", win);
    assert( win == O_WIN );

    board = "..O.O.O..";
    win = isDone(board);
    printf("win sub diagonal O: %d\n", win);
    assert( win == O_WIN );



    
}
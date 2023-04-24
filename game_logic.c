#include "game_logic.h"
#include <stdbool.h>

static int convert_pair_single(int i, int j){
    return i*3 + j;
}

enum win_state isDone(char* board)
{
    int i, j;
    int main_idx, sub_idx, row, col;
    bool main_x = true, main_o = true, sub_x = true, sub_o = true;
    bool row_x, col_x, row_o, col_o;
    for (i = 0; i < 3; i++)
    {
        // main diagonal   
        main_idx = convert_pair_single(i, i);
        main_x &= (board[main_idx] == 'X'? 1 : 0);
        main_o &= (board[main_idx] == 'O'? 1 : 0);
        // sub diagonal
        sub_idx = convert_pair_single(i, 2 - i); 
        sub_x &= (board[sub_idx] == 'X'? 1 : 0);
        sub_o &= (board[sub_idx] == 'O'? 1 : 0);
    }

    if (main_x || sub_x) return X_WIN;
    if (main_o || sub_o) return O_WIN;

    bool isDraw = true;
    for (i = 0; i < 3; i++)
    {
        row_x = true; row_o = true; col_x = true; col_o = true;
        for (j = 0; j < 3; j++)
        {
            row = convert_pair_single(i, j);
            row_x &= (board[row] == 'X'? 1 : 0);
            row_o &= (board[row] == 'O'? 1 : 0); 

            isDraw &= (board[row] != '.'? 1 : 0);

            col = convert_pair_single(j, i);
            col_x &= (board[col] == 'X'? 1 : 0);
            col_o &= (board[col] == 'O'? 1 : 0); 
        }

        //invalid if both player wins
        if ((row_x || col_x) && (row_o || col_o))
            return INVALID; 

        if (row_x || col_x) return X_WIN;
        if (row_o || col_o) return O_WIN;
    }
    if (isDraw) return BOARD_FILLED_DRAW;
    return UNFINISHED;
}

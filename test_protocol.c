#include<stdio.h>
#include<stdlib.h>
#include"protocol.h"

int main (){
    char* buf = malloc(sizeof(char));
    int buf_len = 1;
    int play_len = play(&buf, buf_len, "Long Tran is not a very good name",33);   
    printf("message|play|length %d:\t\t%s\n", play_len, buf);
    free(buf);

    //null name
    buf = malloc(sizeof(char));
    buf_len = 1;
    play_len = play(&buf, buf_len, NULL, 0);   
    printf("message|play|length %d:\t\t%s\n", play_len, buf);
    free(buf); 

    buf = malloc(sizeof(char));
    buf_len = 1;
    int wait_len = wait_game(&buf, buf_len);
    printf("message|wait|length %d:\t\t%s\n", wait_len, buf);
    free(buf);

    buf = malloc(sizeof(char));
    buf_len = 1;
    int begin_len = begin(&buf, buf_len, 'X', "Jotaro", 6);
    printf("message|begin|length %d:\t\t%s\n", begin_len, buf);
    free(buf);
    
    buf = malloc(sizeof(char));
    buf_len = 1;
    pair p = {2, 2};
    int move_len = move(&buf, buf_len, 'X', p);
    printf("message|begin|length %d:\t\t%s\n", begin_len, buf);
    free(buf);

    buf = malloc(sizeof(char));
    buf_len = 1;
    char role = 'X';
    char board[9] = ".........";
    pair p2 = {1, 1};
    board[(p2.x - 1) * 3 + p2.y - 1] = role;
    printf("board is %s\n", board);
    int move_board_len = move_board(&buf, buf_len, role, p2, board);
    printf("message|begin|length %d:\t\t%s\n", begin_len, buf);
    free(buf);

    // invalid message
    buf = malloc(sizeof(char));
    buf_len = 1;
    int invalid_len = invalid(&buf, buf_len, "Warning!!! invalid reason", 25);   
    printf("message|play|length %d:\t\t%s\n", invalid_len, buf);
    free(buf); 
}
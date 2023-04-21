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
    printf("message|move|length %d:\t\t%s\n", move_len, buf);
    free(buf);

    buf = malloc(sizeof(char));
    buf_len = 1;
    char role = 'X';
    char board[9] = ".........";
    pair p2 = {2, 2};
    board[(p2.x - 1) * 3 + p2.y - 1] = role;
    int move_board_len = move_board(&buf, buf_len, role, p2, board);
    printf("message|movd|length %d:\t\t%s\n", move_board_len, buf);
    free(buf);

    // invalid message
    buf = malloc(sizeof(char));
    buf_len = 1;
    int invalid_len = invalid(&buf, buf_len, "Warning!!! invalid reason", 25);   
    printf("message|invl|length %d:\t\t%s\n", invalid_len, buf);
    free(buf); 

    // draw message
    buf = malloc(sizeof(char));
    buf_len = 1;
    int draw_len = draw(&buf, buf_len, SUGGEST);   
    printf("message|draw|length %d:\t\t%s\n", draw_len, buf);
    free(buf); 

    // over message
    buf = malloc(sizeof(char));
    buf_len = 1;
    char* reason = "this is a valid reason for game over!";
    int reason_len = 37;
    int over_len = over(&buf, buf_len, WIN, reason, reason_len);   
    printf("message|over|length %d:\t\t%s\n", over_len, buf);
    free(buf); 
}
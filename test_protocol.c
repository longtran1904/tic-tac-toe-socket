#include<stdio.h>
#include<stdlib.h>
#include"protocol.h"

int main (){
    // generate PLAY message
    char* buf = malloc(sizeof(char));
    int buf_len = 1;
    int play_len = play(&buf, buf_len, "Long Tran is not a very good name",33);   
    printf("message|play|length %d:\t\t%s\n", play_len, buf);

    // parse PLAY message
    message *msg = parse_msg(buf, 34);
    if ( msg != NULL ) {
	printf("code: %s\nname: %s\n", msg->code, msg->name);
    }
    else {
	printf("error: %s\n", get_parse_err_val(parsing_status));
    }
    free(buf);
    free(msg);

    

    // generate PLAY message with null name
    buf = malloc(sizeof(char));
    buf_len = 1;
    play_len = play(&buf, buf_len, NULL, 0);   
    printf("message|play|length %d:\t\t%s\n", play_len, buf);
 
    // parse PLAY message with null name
    msg = parse_msg(buf,1);
    if ( msg != NULL ) {
	printf("code: %s\nname: %s\n", msg->code, msg->name);
    }
    else {
	printf("error: %s\n", get_parse_err_val(parsing_status));
    }
    free(buf);
    free(msg);

// NOTE: send message size as written in message to parse_msg()

    // generate WAIT message
    buf = malloc(sizeof(char));
    buf_len = 1;
    int wait_len = wait_game(&buf, buf_len);
    printf("message|wait|length %d:\t\t%s\n", wait_len, buf);
    free(buf);
    // parse WAIT message 
    msg = parse_msg(buf,1);
    if ( msg != NULL ) {
	printf("code: %s\n", msg->code);
    }
    else {
	printf("error: %s\n", get_parse_err_val(parsing_status));
    }
    free(buf);
    free(msg);


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
    char board[10] = ".........";
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

    // draw message
    buf = malloc(sizeof(char));
    buf_len = 1;
    int draw_len = draw(&buf, buf_len, SUGGEST);   
    printf("message|play|length %d:\t\t%s\n", draw_len, buf);
    free(buf); 

    // over message
    buf = malloc(sizeof(char));
    buf_len = 1;
    char* reason = "this is a valid reason for game over!";
    int reason_len = 37;
    int over_len = over(&buf, buf_len, WIN, reason, reason_len);   
    printf("message|play|length %d:\t\t%s\n", over_len, buf);
    free(buf); 


}

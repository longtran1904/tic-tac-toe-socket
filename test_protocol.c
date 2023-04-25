#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"protocol.h"

int main (){
    // generate PLAY message
    char* buf = malloc(sizeof(char));
    int buf_len = 1;
    int play_len = play(&buf, buf_len, "Long Tran is not a very good name",33);   
    printf("message|play|length %d:\t\t%s\n", play_len, buf);

    // parse PLAY message
    message *msg = parse_msg(buf, play_len-8);
    if ( msg != NULL ) {
	printf("code: %s\nname: %s\n", msg->code, msg->name);
    }
    else {
	printf("error: %s\n", get_msg_info_str(msg_info));
    }
    free(buf);
    free(msg);

    

    // generate PLAY message with null name
    buf = malloc(sizeof(char));
    buf_len = 1;
    play_len = play(&buf, buf_len, NULL, 0);   
    printf("message|play|length %d:\t\t%s\n", play_len, buf);
 
    // parse PLAY message with null name
    msg = parse_msg(buf,0);
    if ( msg != NULL ) {
	printf("code: %s\nname: %s\n", msg->code, msg->name);
    }
    else {
	printf("error: %s\n", get_msg_info_str(msg_info));
    }
    free(buf);
    free(msg);


    // generate WAIT message
    buf = malloc(sizeof(char));
    buf_len = 1;
    int wait_len = wait_game(&buf, buf_len);
    printf("message|wait|length %d:\t\t%s\n", wait_len, buf);
    // parse WAIT message 
    msg = parse_msg(buf,0);
    if ( msg != NULL ) {
	printf("code: %s\n", msg->code);
    }
    else {
	printf("error: %s\n", get_msg_info_str(msg_info));
    }
    free(buf);
    free(msg);


    // generate BEGN message
    buf = malloc(sizeof(char));
    buf_len = 1;
    int begin_len = begin(&buf, buf_len, 'X', "Jotaro", 7); // 7 to account for '\0' for printing
    printf("message|begin|length %d:\t\t%s\n", begin_len, buf);
    // parse BEGN message 
    msg = parse_msg(buf,begin_len-8);
    if ( msg != NULL ) {
	printf("code: %s\nrole: %c\nname: %s\n", msg->code, msg->role, msg->name);
    }
    else {
	printf("error: %s\n", get_msg_info_str(msg_info));
    }
    free(buf);
    free(msg);


    // generate MOVE message
    buf = malloc(sizeof(char));
    buf_len = 1;
    pair p = {2, 2};
    int move_len = move(&buf, buf_len, 'X', p);
    printf("message|move|length %d:\t\t%s\n", move_len, buf);
    // parse MOVE message 
    msg = parse_msg(buf,move_len-7);
    if ( msg != NULL ) {
	printf("code: %s\nrole: %c\nposition: (%d, %d)\n", msg->code, msg->role, msg->position.x, msg->position.y);
    }
    else {
	printf("error: %s\n", get_msg_info_str(msg_info));
    }
    free(buf);
    free(msg);


    // generate MOVD message
    buf = malloc(sizeof(char));
    buf_len = 1;
    char role = 'X';
    char board[10] = ".........";
    pair p2 = {1, 1};
    board[(p2.x - 1) * 3 + p2.y - 1] = role;
    int move_board_len = move_board(&buf, buf_len, role, p2, board);
    printf("message|movd|length %d:\t\t%s\n", move_board_len, buf);
    // parse MOVD message 
    msg = parse_msg(buf,move_board_len-8);
    if ( msg != NULL ) {
	printf("code: %s\nrole: %c\nposition: (%d, %d)\n", msg->code, msg->role, msg->position.x, msg->position.y);
    }
    else {
	printf("error: %s\n", get_msg_info_str(msg_info));
    }
    free(buf);
    free(msg);


    // generate INVL message
    buf = malloc(sizeof(char));
    buf_len = 1;
    int invalid_len = invalid(&buf, buf_len, "Warning!!! invalid reason", 26); // len is 26 so includes '\0' for printing
    printf("message|invl|length %d:\t\t%s\n", invalid_len, buf);
    // parse INVL message 
    msg = parse_msg(buf,invalid_len-8);
    if ( msg != NULL ) {
	printf("code: %s\nreason: %s\n", msg->code, msg->reason);
    }
    else {
	printf("error: %s\n", get_msg_info_str(msg_info));
    }
    free(buf);
    free(msg);


    // generate DRAW message
    buf = malloc(sizeof(char));
    buf_len = 1;
    int draw_len = draw(&buf, buf_len, SUGGEST);   
    printf("message|draw|length %d:\t\t%s\n", draw_len, buf);
    // parse DRAW message 
    msg = parse_msg(buf,draw_len-7);
    if ( msg != NULL ) {
	printf("code: %s\nmessage: %c\n", msg->code, msg->msg);
    }
    else {
	printf("error: %s\n", get_msg_info_str(msg_info));
    }
    free(buf);
    free(msg);


    // generate OVER message
    buf = malloc(sizeof(char));
    buf_len = 1;
    char* reason = "this is a valid reason for game over!";
    int reason_len = 38; // added 1 to make str for printing
    int over_len = over(&buf, buf_len, WIN, reason, reason_len);   
    printf("message|over|length %d:\t\t%s\n", over_len, buf);
    // parse OVER message 
    msg = parse_msg(buf,over_len-8);
    if ( msg != NULL ) {
	printf("code: %s\noutcome: %c\nreason: %s\n", msg->code, msg->outcome, msg->reason );
    }
    else {
	printf("error: %s\n", get_msg_info_str(msg_info));
    }
    free(buf);
    free(msg);


    // TESTING both get_msg_shift_buf() and parse_msg()
    buf = malloc(sizeof(char)*72);
    char buf_sim_str[73] = "OVER|40|W|this is a valid reason for game over!|MOVD|16|X|1,1|X........|";
    memcpy(buf, buf_sim_str, 72);
    printf("simulated buffer: %s\n", buf_sim_str);

    int buf_start_index = 0;
    int msg_size = 0;
    char *message0 = grab_msg_shift_buf( buf, 72, &msg_size, &buf_start_index);

    if ( message0 == NULL ) {
	printf("error: %s\n", get_msg_info_str(msg_info));
    }
    else {
	char *message0_str = malloc(9+msg_size);
	memcpy(message0_str, message0, 8+msg_size); 
	message0_str[8+msg_size] = '\0';
	printf("grabbed message: %s\n", message0_str);

	buf[buf_start_index] = '\0';
	printf("shifted-buf: %s\n", buf);

	msg = parse_msg(message0, msg_size);
	if ( msg != NULL ) {
	    printf("code: %s\noutcome: %c\nreason: %s\n", msg->code, msg->outcome, msg->reason);
	}
	else {
	    printf("error: %s\n", get_msg_info_str(msg_info));
	}

	free(msg);
	free(message0_str);
    }

    char *message1 = grab_msg_shift_buf( buf, 24, &msg_size, &buf_start_index);

    if ( message1 == NULL ) {
	printf("grab_msg_shift_buf() error: %s\n", get_msg_info_str(msg_info));
    }
    else {
	char *message1_str = malloc(9+msg_size);
	memcpy(message1_str, message1, 8+msg_size); 
	message1_str[8+msg_size] = '\0';
	printf("grabbed message: %s\n", message1_str);

	msg = parse_msg(message1, msg_size);
	if ( msg != NULL ) {
	    char brd[10];
	    memcpy(brd, msg->board, 9);
	    brd[9] = '\0';
	    printf("code: %s\nrole: %c\nposition: (%d, %d)\nboard: %s\n", msg->code, msg->role, msg->position.x, msg->position.y, brd);
	}
	else {
	    printf("parse_msg() error: %s\n", get_msg_info_str(msg_info));
	}

	free(msg);
	free(message1_str);
    }

    free(message0);
    free(message1);
    free(buf);
}

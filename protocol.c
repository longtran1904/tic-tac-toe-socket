#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "protocol.h"
#define CODELEN 5
#define BOARDLEN 9

#ifndef DEBUG
    #define DEBUG 0
#endif

static int line_pos;

static int append(char** string, int* len, char* append, int append_len){
    while (append_len + line_pos >= *len){
        (*len) *= 8;
        *string = (char*) realloc(*string, *len);
    }
    strncpy((*string)+line_pos, append, append_len);
    return line_pos + append_len;
}

static int NumToWord(char** word, int word_len, int number){
    if (number == 0){
        char zero = '0';
        line_pos = append(word, &word_len, &zero, 1);
    }
    while (number){
        char num = number % 10 + '0';
        line_pos = append(word, &word_len, &num, 1);
        number /= 10;
    }

    // reverse string
    for (int i = 0; i < line_pos/2; i++)
    {
        char tmp = (*word)[i];
        (*word)[i] = (*word)[line_pos - 1 - i];
        (*word)[line_pos - 1 - i] = tmp;
    }

    char end = '|';
    line_pos = append(word, &word_len, &end, 1);

    return line_pos;
}
int play(char** buf, int buf_len, char* name, int length){
    char* code = "PLAY|";

    int msg_length = length + (length > 0 ? 1 : 0);
    printf("msg length: %d\n", msg_length);

    char* numBuf = malloc(sizeof(char));
    int numBuf_len = 1;
    line_pos = 0;
    
    // get string of length of message
    int num_len = NumToWord(&numBuf, numBuf_len, msg_length);

    if (DEBUG) printf("numBuf | length %d: \n%s\n", num_len, numBuf);
    
    // assign to buf 
    line_pos = 0;
    line_pos = append(buf, &buf_len, code, CODELEN);
    if (DEBUG) printf("add code -> line_pos: %d\n", line_pos);
    line_pos = append(buf, &buf_len, numBuf, num_len);
    if (DEBUG) printf("add length -> line_pos: %d\n", line_pos);
    if (msg_length > 0) {
        line_pos = append(buf, &buf_len, name, length);
        if (DEBUG) printf("add name -> line_pos %d\nbuf_len -> %d\n", line_pos, buf_len);
        char end = '|';
        line_pos = append(buf, &buf_len, &end, 1);
    }   

    free(numBuf);
    
    if (line_pos < buf_len) (*buf)[line_pos] = '\0';
    return line_pos;
}

int wait_game(char** buf, int buf_len){
    char code[7] = "WAIT|0|";
    line_pos = 0;
    line_pos = append(buf, &buf_len, code, 7);

    if (line_pos <= buf_len) (*buf)[line_pos] = '\0';
    return line_pos;
}

int resign(char** buf, int buf_len){
    char code[7] = "RSGN|0|";
    line_pos = 0;
    line_pos = append(buf, &buf_len, code, 7);

    if (line_pos <= buf_len) (*buf)[line_pos] = '\0';
    return line_pos;
}

int begin(char** buf, int buf_len, char role, char* name, int length_name){
    char* code = "BEGN|";
    int msg_length = 2 + length_name + (length_name ? 1 : 0);

    char* numBuf = malloc(sizeof(char));
    int numBuf_len = 1;    
    line_pos = 0;

    // get string of length of message
    int num_len = NumToWord(&numBuf, numBuf_len, msg_length); 

    char roles[2];
    roles[0] = role;
    roles[1] = '|';

    line_pos = 0;
    line_pos = append(buf, &buf_len, code, CODELEN);
    line_pos = append(buf, &buf_len, numBuf, num_len);
    line_pos = append(buf, &buf_len, roles, 2);
    if (msg_length > 0){
        line_pos = append(buf, &buf_len, name, length_name);
        char end = '|';
        line_pos = append(buf, &buf_len, &end, 1);
    }    

    free(numBuf);

    if (line_pos < buf_len) (*buf)[line_pos] = '\0';
    return line_pos;
}
int move(char** buf, int buf_len, char role, pair p){
    if (role != 'X' && role != 'O') {
        printf("Wrong role character");
        return -1;
    }
    if (p.x < 1 || p.x > 3 || p.y < 1 || p.y > 3){
        printf("position pair out of range [3,3]!");
        return -1;
    }
    char* code = "MOVE|";
    char roles[2] = {role, '|'};
    char pos[4] = {p.x + '0', ',', p.y + '0', '|'};

    int msg_length = 2 + 4; // role = 2 + position + 4
    char* numBuf = malloc(sizeof(char));
    int numBuf_len = 1;
    line_pos = 0;
    // get string of length of message
    int num_len = NumToWord(&numBuf, numBuf_len, msg_length);

    // appending to get protocol message
    line_pos = 0;
    line_pos = append(buf, &buf_len, code, CODELEN); // code
    line_pos = append(buf, &buf_len, numBuf, num_len); // length field
    line_pos = append(buf, &buf_len, roles, 2);
    line_pos = append(buf, &buf_len, pos, 4);
    
    free(numBuf);

    if (line_pos < buf_len) (*buf)[line_pos] = '\0';
    return line_pos;
}

int move_board(char** buf, int buf_len, char role, pair p, char* board){
    if (role != 'X' && role != 'O') {
        printf("Wrong role character");
        return -1;
    }
    if (p.x < 1 || p.x > 3 || p.y < 1 || p.y > 3){
        printf("position pair out of range [3,3]!");
        return -1;
    }
    char* code = "MOVD|";
    char roles[2] = {role, '|'};
    char pos[4] = {p.x + '0', ',', p.y + '0', '|'};

    int msg_length = 2 + 4 + BOARDLEN + 1; // role = 2 + pos = 4 + board = 10
    char* numBuf = malloc(sizeof(char));
    int numBuf_len = 1;
    line_pos = 0;
    // get string of length of message
    int num_len = NumToWord(&numBuf, numBuf_len, msg_length);

    // appending to get protocol message
    line_pos = 0;
    line_pos = append(buf, &buf_len, code, CODELEN); // code
    line_pos = append(buf, &buf_len, numBuf, num_len); // length field
    line_pos = append(buf, &buf_len, roles, 2);
    line_pos = append(buf, &buf_len, pos, 4);
    line_pos = append(buf, &buf_len, board, BOARDLEN);
    char end = '|';
    line_pos = append(buf, &buf_len, &end, 1);
    
    free(numBuf);
    if (line_pos < buf_len) (*buf)[line_pos] = '\0';
    return line_pos;
    
}

int invalid(char** buf, int buf_len, char* reason, int reason_length){
   char* code = "INVL|";
    int msg_length =  reason_length + (reason_length ? 1 : 0);

    char* numBuf = malloc(sizeof(char));
    int numBuf_len = 1;    
    line_pos = 0;

    // get string of length of message
    int num_len = NumToWord(&numBuf, numBuf_len, msg_length); 

    line_pos = 0;
    line_pos = append(buf, &buf_len, code, CODELEN);
    line_pos = append(buf, &buf_len, numBuf, num_len);
    if (msg_length > 0){
        line_pos = append(buf, &buf_len, reason, reason_length);
        char end = '|';
        line_pos = append(buf, &buf_len, &end, 1);
    }    

    free(numBuf);

    if (line_pos < buf_len) (*buf)[line_pos] = '\0';
    return line_pos; 
}

int draw(char** buf, int buf_len, enum draw_state state){
    char code[5] = "DRAW|";
    char length[2] = "2|";
    char mode[2];
    switch (state){
        case SUGGEST:
            mode[0] = 'S'; mode[1] = '|'; break;
        case ACCEPT:
            mode[0] = 'A'; mode[1] = '|'; break;
        case REJECT:
            mode[0] = 'R'; mode[1] = '|'; break;
    }    

    line_pos = 0;
    line_pos = append(buf, &buf_len, code, CODELEN);
    line_pos = append(buf, &buf_len, length, 2);
    line_pos = append(buf, &buf_len, mode, 2);

    if (line_pos < buf_len)
        (*buf)[line_pos] = '\0';

    return line_pos;


}
int over(char** buf, int buf_len, enum game_state state, char* win_reason, int win_length){
    char* code = "OVER|";
    
    char* mode;
    switch (state){
        case WIN:
            mode = "W|";
            break;
        case LOSE:
            mode = "L|";
            break;
        case DRAW:
            mode = "D|";
            break;
    }

    int msg_length = 2 + win_length + (win_length > 0 ? 1 : 0);
    char* numBuf = malloc(sizeof(char));
    int numBuf_len = 1;    
    line_pos = 0;

    // get string of length of message
    int num_len = NumToWord(&numBuf, numBuf_len, msg_length); 

    line_pos = 0;
    line_pos = append(buf, &buf_len, code, CODELEN);
    line_pos = append(buf, &buf_len, numBuf, num_len);
    line_pos = append(buf, &buf_len, mode, 2);
    line_pos = append(buf, &buf_len, win_reason, win_length);
    char end = '|';
    line_pos = append(buf, &buf_len, &end, 1);

    if (line_pos < buf_len)
        (*buf)[line_pos] = '\0';

    return line_pos;
}

int send_message( int sock, char *msg_buf, int msg_len ) {

    // print sending messages
    char *printable_buf = malloc(sizeof(char) * (msg_len + 1));
    memcpy(printable_buf, msg_buf, msg_len);
    printable_buf[msg_len] = '\0';
    printf("sending to sock %d| message %s\n", sock, printable_buf);
    // print sending messages

    int write_bytes = write(sock, msg_buf, msg_len);

    if (write_bytes == -1){
	perror("write");
	return -1; // failure
    }

    printf("write %d bytes\n", write_bytes);
    return 0; // success
}

void populate_play(char *fields, int fields_len, message *result) {
    msg_info = ALL_GOOD;
    int count = 0;

    if ( fields_len < 2 ) {
	msg_info = BAD_FORMAT;
	return;
    }

    // find next '|'
    while ( count < fields_len && fields[count] != '|' ) { count++; }
    if ( count < fields_len-1 || fields_len-1 > 100 ) { // there is a bar in the name OR the name is too long
	msg_info = BAD_NAME_FLD;
	return;
    } 

    memcpy(result->name, fields, fields_len-1);
    result->name[fields_len-1] = '\0';
    result->name_len = fields_len-1;
}

void set_role( char *fields, int fields_len, message *result ) {
    msg_info = ALL_GOOD;

    if ( fields[1] != '|' ) {
	msg_info = BAD_FORMAT;
	return;
    }

    char role = fields[0];

    if ( role == 'X' || role == 'O' ) {
	result->role = role;
    } // role char is NOT 'X' or 'O'
    else {
	msg_info = BAD_ROLE_FLD;
	return;
    }
}

void populate_begn(char *fields, int fields_len, message *result) {
    msg_info = ALL_GOOD;
    int count = 2;

    if ( fields_len < 4 ) {
	msg_info = BAD_FORMAT;
	return;
    }

    set_role( fields, fields_len, result );
    if ( msg_info != ALL_GOOD ) return; // set_role failed

    // find next '|'
    while ( count < fields_len && fields[count] != '|' ) { count++; }
    if ( count < fields_len-1 || (fields_len-3) > 100 ) { // there is a bar in the name OR the name is too long
	msg_info = BAD_NAME_FLD;
	return;
    } 

    memcpy(result->name, fields+2, fields_len-3);
    result->name[fields_len-3] = '\0';
    result->name_len = fields_len-3;
}

void set_pos( char *fields, int fields_len, message *result ) {
    if ( fields[3] != ',' ) {
	msg_info = BAD_FORMAT;
	return;
    }

    int pos0 = (int) (fields[2] - '0'); 
    int pos1 = (int) (fields[4] - '0');
    if ( pos0 < 1 || pos0 > 3 || pos1 < 1 || pos1 > 3 ) {
	msg_info = BAD_POSITION_FLD;
	return; // incorrect ints present
    }

    result->position.x = pos0;
    result->position.y = pos1;
}


void populate_move(char *fields, int fields_len, message *result) {
    msg_info = ALL_GOOD;

    if ( fields_len != 6 ) {
	msg_info = BAD_FORMAT;
	return; // incorrect message length
    }

    set_role( fields, fields_len, result );
    if ( msg_info != ALL_GOOD ) return; // set_role failed

    set_pos( fields, fields_len, result );
}
    
void populate_movd(char *fields, int fields_len, message *result) {

    // see fields - DELETE WHEN DEPLOY
    char* printable_fields = malloc(sizeof(char) * (fields_len+1));
    memcpy(printable_fields, fields, fields_len);
    printable_fields[fields_len] = '\0';
    printf("fields from MOVD |%s\n", printable_fields);
    free(printable_fields);
    // see fields - DELETE WHEN DEPLOY 

    msg_info = ALL_GOOD;

    if ( fields_len != 16 ) {
	msg_info = BAD_FORMAT;
	return; // incorrect message length
    }

    set_role( fields, fields_len, result );
    if ( msg_info != ALL_GOOD ) return; // set_role failed

    set_pos( fields, fields_len, result );
    if ( msg_info != ALL_GOOD ) return; // set_pos failed

    int i = 6, j = 0;
    while ( i < fields_len-1 ) {
	char c = fields[i++];
	if ( c != 'X' && c != 'O' && c != '.' ) {
	    msg_info = BAD_BOARD_FLD;
	    return; // incorrect chars present
	}
	result->board[j++] = c;
    }
}

void populate_invl(char *fields, int fields_len, message *result) {
    msg_info = ALL_GOOD;

    if ( fields_len < 2 ) {
	msg_info = BAD_FORMAT;
	return;
    }

    if ( fields_len-1 > 100 ) {
	msg_info = BAD_REASON_FLD;
	return; // reason is too long
    } 

    memcpy(result->reason, fields, fields_len-1);
    result->reason[fields_len-1] = '\0';
    result->reason_len = fields_len-1;
}

void populate_draw(char *fields, int fields_len, message *result) {
    msg_info = ALL_GOOD;

    if ( fields_len != 2 ) {
	msg_info = BAD_FORMAT;
	return; // incorrect length
    }

    char c = fields[0];

    if ( c != 'S' && c != 'A' && c != 'R' ) {
	msg_info = BAD_MSG_FLD;
	return;	// incorrect chars present
    }

    result->msg = c;
}

void populate_over(char *fields, int fields_len, message *result) {
    msg_info = ALL_GOOD;

    if ( fields_len < 3 ) {
	msg_info = BAD_FORMAT;
	return;
    }

    char c = fields[0];

    if ( c != 'W' && c != 'L' && c != 'D' ) {
	msg_info = BAD_OUTCOME_FLD;
	return;	// incorrect chars present
    }

    if ( fields[1] != '|' ) {
	msg_info = BAD_FORMAT;
	return;
    }

    result->outcome = c;

    if ( fields_len-3 > 100 ) {
	msg_info = BAD_REASON_FLD;
	return; // reason is too long
    } 

    memcpy(result->reason, fields+2, fields_len-3);
    result->reason[fields_len-3] = '\0';
}

char *get_msg_info_str( message_information stat ) {
    switch (stat) {
	case ALL_GOOD: return "ALL_GOOD";
	case UNEXPECTED: return "UNEXPECTED";
	case INCOMPLETE_MSG: return "INCOMPLETE_MSG";
	case BAD_FORMAT: return "BAD_FORMAT";
	case BAD_SIZE_FLD: return "BAD_SIZE_FLD";
	case BAD_CODE_FLD: return "BAD_CODE_FLD";
	case BAD_NAME_FLD: return "BAD_NAME_FLD";
	case BAD_ROLE_FLD: return "BAD_ROLE_FLD";
	case BAD_POSITION_FLD: return "BAD_POSITION_FLD";
	case BAD_BOARD_FLD: return "BAD_BOARD_FLD";
	case BAD_REASON_FLD: return "BAD_REASON_FLD";
	case BAD_MSG_FLD: return "BAD_MSG_FLD";
	case BAD_OUTCOME_FLD: return "BAD_OUTCOME_FLD";
    }
}

// checks that msg_size is what it claims to be and that message ends in '|' 
// returns a pointer to said message if the above holds true and sets msg_size and buf_offset
// IF the above fails: returns NULL and sets msg_info to one of 'enum message_information'
// NOTE: if the message is incomplete, returns NULL and sets msg_info to INCOMPLETE_MSG 
// to signify that we need to call read again to recieve a complete message
// !!! IF WE RECIEVE INCOMPLETE_MSG, WE MUST COMPOUND buf_len WITH EACH CALL !!!
// NOTE: the pointer returned must be freed
char *grab_msg_shift_buf( char *buf, int buf_len, int *msg_size, int *buf_offset) {
    msg_info = ALL_GOOD;
    *buf_offset = 0;

    // there is less than one message in buf
    if ( buf_len < 7 ) {
	msg_info = INCOMPLETE_MSG;
	*buf_offset = buf_len;
	return NULL;
    }

    if ( buf[4] != '|' ) {
	msg_info = BAD_FORMAT;
	return NULL;
    }

    // get message size
    int count = 5;
    while ( (count < buf_len) && (buf[count] != '|') ) { count++; }
    
    //check if we have complete size field
    if ( count == buf_len && buf[count] != '|' ) {
	if ( count > 7 ) { // invalid length: must be < 255 -> 3 or less chars
	    msg_info = BAD_SIZE_FLD;
	    return NULL;
	} // length field could be incomplete
	else {
	    msg_info = INCOMPLETE_MSG;
	    *buf_offset = buf_len;
	    return NULL;
	}
    }

    // grab size
    int num_chars_sz = count-5;
    char size_str[num_chars_sz+1];
    memcpy(size_str, buf+5, num_chars_sz);
    size_str[num_chars_sz] = '\0';
    int size = atoi(size_str);
    *msg_size = size;

    int expct_msg_len = 5+num_chars_sz+1+size;

    // there is less than one message in buf
    if ( expct_msg_len > buf_len ) {
	msg_info = INCOMPLETE_MSG;
	*buf_offset = buf_len;
	return NULL;
    } // there is at least one message in buf 
    else {
	// check if message ends in '|'
	if ( buf[expct_msg_len-1] != '|' ) {
	    msg_info = BAD_FORMAT;
	    return NULL;
	}

	char *msg = malloc(expct_msg_len);
	memcpy(msg, buf, expct_msg_len);
	// there is exactly one message in buffer
	if ( expct_msg_len == buf_len ) {
	    return msg;
	} // there is more than one message in buf: we need to shift buf 
	else {
	    memmove(buf, buf+expct_msg_len, buf_len-expct_msg_len);
	    *buf_offset = buf_len-expct_msg_len; 
	    return msg;
	}
    }
}

// parses message into 'struct message' and returns a pointer to said struct
// NOTE: the pointer returned must be freed
// IF parsing fails: returns NULL and sets msg_info to one of 'enum message_information'
message *parse_msg(char* msg_inp, int msg_size) {
    msg_info = ALL_GOOD;

    printf("msg_size: %d\n", msg_size);

    message *result = malloc( sizeof(message) );
    if ( msg_inp[4] == '|' ) {
	memcpy(result->code, msg_inp, 4);
	result->code[4] = '\0';
    } // invalid message format
    else {
	msg_info = BAD_FORMAT;
	free(result);
	return NULL;
    }

    char msg_code[5];
    strcpy(msg_code, result->code);

    if ( strcmp(msg_code, "RSGN") == 0 || strcmp(msg_code, "WAIT") == 0 ) {
	return result;
    }

    // skip the size field
    int count = 5;
    while ( msg_inp[count] != '|' ) { count++; }
    char *msg_fields = &msg_inp[count+1];

    if ( strcmp(msg_code, "PLAY") == 0 ) {
	populate_play(msg_fields, msg_size, result);
    } 
    else if ( strcmp(msg_code, "BEGN") == 0 ) {
	populate_begn(msg_fields, msg_size, result);
    }
    else if ( strcmp(msg_code, "MOVE") == 0 ) {
	populate_move(msg_fields, msg_size, result);
    }
    else if ( strcmp(msg_code, "MOVD") == 0 ) {
	populate_movd(msg_fields, msg_size, result);
    }
    else if ( strcmp(msg_code, "INVL") == 0 ) {
	populate_invl(msg_fields, msg_size, result);
    }
    else if ( strcmp(msg_code, "DRAW") == 0 ) {
	populate_draw(msg_fields, msg_size, result);
    }
    else if ( strcmp(msg_code, "OVER") == 0 ) {
	populate_over(msg_fields, msg_size, result);
    } // incorrect code field
    else {
	msg_info = BAD_CODE_FLD;
    }

    if ( msg_info == ALL_GOOD ) {
	return result;
    } // there was an error parsing message into struct

    // msg_info when error
    printf("msg_info: %d\n", msg_info);
    return NULL;
}

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "protocol.h"
#define CODELEN 5
#define BOARDLEN 9

#ifndef DEBUG
    #define DEBUG 0
#endif

static int line_pos;

static int append(char** string, int* len, char* append, int append_len){
    while (append_len + line_pos >= *len){
        *len *= 2;
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

    int msg_length = CODELEN + 2 + 4;
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

    int msg_length = CODELEN + 2 + 4;
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
        case SUGGEST:
            mode = "W|";
            break;
        case ACCEPT:
            mode = "L|";
            break;
        case REJECT:
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

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "protocol.h"
#define CODELEN 5

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
    return line_pos;


}

int wait_game(char** buf, int buf_len){
    char code[7] = "WAIT|0|";
    line_pos = 0;
    line_pos = append(buf, &buf_len, code, 7);
    return line_pos;
}
int begin(char** buf, int buf_len, char role, char* name, int length_name);
int move(char** buf, int buf_len, char role, pair p);
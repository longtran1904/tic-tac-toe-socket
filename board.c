#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdbool.h>

bool isDone(char game[9]){
    int i;
    for (i = 0; i < 9; i++)
        if (game[i] == ' ') return false;
    
    return true;
}

void print_board(char game[9]){
    int i,j, count = 0;
    for (i = 0; i < 7; i++){
        for (j = 0; j < 7; j++)
            if (i % 2 == 0) printf("-");
            else {
                if (j % 2 == 0) printf("|");
                else {
                    if (game[count] != ' ')
                        printf("%c", game[count++]);
                    else printf("%d", ++count);
                }
            }    
        printf("\n");
    }
}

int main(){
    char game[9];
    int i, j;

    // TODO: Retrieve game[9] from server
    for (i = 0; i < 9; i++)
        game[i] = ' ';

    printf("Welcome to Tic-Tac-Toe!!!\n");    
    
    char buf;
    bool validMove = true;

    do {
        if (!validMove) {
            printf("Wrong format! Type a number from 1 to 9\n");
            validMove = true;
        }
        print_board(game);
        printf("Please choose cell number to make a move (1 to 9)\n");
        if (read(STDIN_FILENO, &buf, 1) > 0){
            if (buf - '0' < 0 || buf - '0' > 9){
                validMove = false;
                continue;
            }
            // TODO: send move signal to server
            game[(buf - '0') - 1] = 'X';
        }
    } while (!isDone(game));
    return EXIT_SUCCESS;
}
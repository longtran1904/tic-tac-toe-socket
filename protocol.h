typedef struct pair{
    //move position x,y
    int x;
    int y;
} pair;
enum draw_state {SUGGEST, ACCEPT, REJECT};
enum game_state {WIN, LOSE, DRAW};

// Send Messages
// write formatted messsage to buffer,
// returns length written to buffer
    int play(char** buf, int buf_len, char* name, int length);
    int wait_game(char** buf, int buf_len);
    int begin(char** buf, int buf_len, char role, char* name, int length_name);
    int move(char** buf, int buf_len, char role, pair p);
    // Board as string of 9 chars (9 cells of the game)
    int move_board(char** buf, int buf_len, char role, pair p, char* board); 
    int invalid(char** buf, int buf_len, char* reason, int reason_length);

    // Game behaviors
    int draw(char** buf, int buf_len, enum draw_state);
    int over(char** buf, int buf_len, enum game_state);

// Deciphering Messages
    void decipher(char* msg, int msg_len);
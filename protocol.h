typedef struct pair{
    //move position x,y
    int x;
    int y;
} pair;
enum draw_state {SUGGEST, ACCEPT, REJECT};
enum game_state {WIN, LOSE, DRAW};

// Send messages
    char* play(char* name, int length);
    char* wait();
    char* begin(char role, char* name, int length_name);
    char* move(char role, pair p);
    // Board as string of 9 chars (9 cells of the game)
    char* move_board(char role, pair p, char* board); 
    char* invalid(char* reason, int reason_length);

    // Game behaviors
    char* draw(enum draw_state);
    char* over(enum game_state);

// Deciphering Messages
    void decipher(char*);
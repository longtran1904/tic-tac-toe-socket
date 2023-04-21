typedef struct pair{
    //move position x,y
    int x;
    int y;
} pair;

typedef struct message {
    char code[5]; // NOTE: code is stored as a string
    char name[101]; // NOTE: name is stored as a string -> can handle names of length <= 70 chars
    char role;
    pair position;
    char board[9]; // NOTE: board is NOT saved as a string
    char reason[101]; // TODO: ensure this is set to agreed upon max length of "reason" 
    char msg;
    char outcome;
} message;

typedef enum {
    SUCCESS,
    BAD_FORMAT, 
    BAD_CODE_FLD,
    BAD_NAME_FLD,
    BAD_ROLE_FLD,
    BAD_POSITION_FLD,
    BAD_BOARD_FLD,
    BAD_REASON_FLD,
    BAD_MSG_FLD,
    BAD_OUTCOME_FLD
} msg_parse_stat;

msg_parse_stat parsing_status;

enum draw_state {SUGGEST, ACCEPT, REJECT};
enum game_state {WIN, LOSE, DRAW};

// Send Messages
// write formatted message to buffer,
// returns length written to buffer
int play(char** buf, int buf_len, char* name, int length);
int wait_game(char** buf, int buf_len);
int begin(char** buf, int buf_len, char role, char* name, int length_name);
int move(char** buf, int buf_len, char role, pair p);
// Board as string of 9 chars (9 cells of the game)
int move_board(char** buf, int buf_len, char role, pair p, char* board); 
int invalid(char** buf, int buf_len, char* reason, int reason_length);

// Game behaviors
int draw(char** buf, int buf_len, enum draw_state state);
int over(char** buf, int buf_len, enum game_state state, char* win_reason, int win_length);

// Parsing Messages into message struct

// parses message into 'struct message' and returns a pointer to said struct
// NOTE: the pointer returned must be freed
// IF parsing fails: returns NULL and sets parsing_status to one of 'enum msg_parse_stat'
// NOTE: msg_size should be the size specified in the message itself
message *parse_msg(char* msg_inp, int msg_size);
void populate_play(char *fields, int fields_len, message *result);
void set_role( char *fields, int fields_len, message *result );
void populate_begn(char *fields, int fields_len, message *result);
void set_pos( char *fields, int fields_len, message *result );
void populate_move(char *fields, int fields_len, message *result);
void populate_movd(char *fields, int fields_len, message *result);
void populate_invl(char *fields, int fields_len, message *result);
void populate_draw(char *fields, int fields_len, message *result);
void populate_over(char *fields, int fields_len, message *result);
void populate_over(char *fields, int fields_len, message *result);

// Printing parsing errors
char *get_parse_err_val( msg_parse_stat stat );

// returns a pointer to a single message from buf and sets msg_size appropriately
// IF fails: returns NULL and sets parsing_status to one of 'enum msg_parse_stat'
char *grab_msg_shift_buf( char *buf, int buf_len, int *msg_size );

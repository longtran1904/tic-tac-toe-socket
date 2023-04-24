enum win_state {X_WIN, O_WIN, BOARD_FILLED_DRAW, UNFINISHED, INVALID};

static int convert_pair_single(int i, int j);
enum win_state isDone(char* board);

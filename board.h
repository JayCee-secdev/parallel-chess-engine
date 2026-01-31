#ifndef BOARD_H
#define BOARD_H

#define BOARD_SIZE 8
#define MAX 1
#define MIN -1

struct config {
    char board[BOARD_SIZE][BOARD_SIZE];
    int turn; // MAX = White, MIN = Black
};

// Board Initialization and Display
void init_board(struct config *board);
void print_board(struct config *board);

// Move Management
int add_move(struct config *conf, int from_x, int from_y, int to_x, int to_y, struct config *successors, int *num_successors);
int player_move(struct config *board, char *from, char *to);
int is_game_over(struct config *board);

// Successor Generation
void generate_successors(struct config *conf, struct config *successors, int *num_successors);

// Move Recommendation
void recommend_moves(struct config *board);

//Piece-Specific Move Validation
int validate_rook_move(struct config *board, int from_x, int from_y, int to_x, int to_y);
int validate_bishop_move(struct config *board, int from_x, int from_y, int to_x, int to_y);
int validate_queen_move(struct config *board, int from_x, int from_y, int to_x, int to_y);
int validate_knight_move(struct config *board, int from_x, int from_y, int to_x, int to_y);
int validate_king_move(struct config *board, int from_x, int from_y, int to_x, int to_y);
int validate_pawn_move(struct config *board, int from_x, int from_y, int to_x, int to_y);


// Piece-Specific Move Generation
void generate_pawn_moves(struct config *conf, int x, int y, struct config *successors, int *num_successors);
void generate_king_moves(struct config *conf, int x, int y, struct config *successors, int *num_successors);
void generate_knight_moves(struct config *conf, int x, int y, struct config *successors, int *num_successors);
void generate_bishop_moves(struct config *conf, int x, int y, struct config *successors, int *num_successors);
void generate_rook_moves(struct config *conf, int x, int y, struct config *successors, int *num_successors);
void generate_queen_moves(struct config *conf, int x, int y, struct config *successors, int *num_successors);

#endif

#include "board.h"
#include "engine.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <omp.h> // For parallel processing

// Initialize the board with standard chess positions
void init_board(struct config *board) {
    memset(board->board, ' ', sizeof(board->board));
    for (int i = 0; i < BOARD_SIZE; i++) {
        board->board[1][i] = 'p'; // Black pawns
        board->board[6][i] = 'P'; // White pawns
    }
    // Set up the rest of the pieces
    char white_row[] = "RNBQKBNR";
    char black_row[] = "rnbqkbnr";
    memcpy(board->board[0], black_row, BOARD_SIZE);
    memcpy(board->board[7], white_row, BOARD_SIZE);
    board->turn = MAX; // White starts
}

// Print the board to the console
void print_board(struct config *board) {
    printf("\n  a b c d e f g h\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%d ", 8 - i);
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf("%c ", board->board[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Recommend moves for the user
void recommend_moves(struct config *board) {
    struct config successors[100];
    int num_successors = 0;

    // Generate all valid moves
    generate_successors(board, successors, &num_successors);

    if (num_successors == 0) {
        printf("No valid moves available.\n");
        return;
    }

    // Evaluate all moves in parallel
    int scores[100];
    #pragma omp parallel for
    for (int i = 0; i < num_successors; i++) {
        scores[i] = evaluate_board(&successors[i]);
    }

    // Find the top moves
    int best_index = 0;
    int second_best_index = -1;
    for (int i = 1; i < num_successors; i++) {
        if (scores[i] > scores[best_index]) {
            second_best_index = best_index;
            best_index = i;
        } else if (second_best_index == -1 || scores[i] > scores[second_best_index]) {
            second_best_index = i;
        }
    }

    // Display the top recommendations
    printf("Move recommendations:\n");
    printf("1. Best move (Score: %d):\n", scores[best_index]);
    print_board(&successors[best_index]);
    if (second_best_index != -1) {
        printf("2. Second best move (Score: %d):\n", scores[second_best_index]);
        print_board(&successors[second_best_index]);
    }
}

// Generate all successors efficiently
void generate_successors(struct config *conf, struct config *successors, int *num_successors) {
    *num_successors = 0; // Reset the count

    #pragma omp parallel for collapse(2)
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            char piece = conf->board[i][j];

            if ((conf->turn == MAX && piece >= 'A' && piece <= 'Z') ||
                (conf->turn == MIN && piece >= 'a' && piece <= 'z')) {
                int local_successors = 0;
                struct config local_configs[50];
                switch (piece) {
                    case 'P': case 'p': generate_pawn_moves(conf, i, j, local_configs, &local_successors); break;
                    case 'K': case 'k': generate_king_moves(conf, i, j, local_configs, &local_successors); break;
                    case 'Q': case 'q': generate_queen_moves(conf, i, j, local_configs, &local_successors); break;
                    case 'B': case 'b': generate_bishop_moves(conf, i, j, local_configs, &local_successors); break;
                    case 'N': case 'n': generate_knight_moves(conf, i, j, local_configs, &local_successors); break;
                    case 'R': case 'r': generate_rook_moves(conf, i, j, local_configs, &local_successors); break;
                }

                #pragma omp critical
                {
                    for (int k = 0; k < local_successors; k++) {
                        successors[*num_successors] = local_configs[k];
                        (*num_successors)++;
                    }
                }
            }
        }
    }
}

// Generate moves for specific pieces
void generate_king_moves(struct config *conf, int x, int y, struct config *successors, int *num_successors) {
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx != 0 || dy != 0) {
                add_move(conf, x, y, x + dx, y + dy, successors, num_successors);
            }
        }
    }
}

void generate_knight_moves(struct config *conf, int x, int y, struct config *successors, int *num_successors) {
    int moves[8][2] = {
        {2, 1}, {2, -1}, {-2, 1}, {-2, -1},
        {1, 2}, {1, -2}, {-1, 2}, {-1, -2}
    };
    for (int i = 0; i < 8; i++) {
        add_move(conf, x, y, x + moves[i][0], y + moves[i][1], successors, num_successors);
    }
}

void generate_bishop_moves(struct config *conf, int x, int y, struct config *successors, int *num_successors) {
    for (int dx = -1; dx <= 1; dx += 2) {
        for (int dy = -1; dy <= 1; dy += 2) {
            for (int step = 1; step < BOARD_SIZE; step++) {
                if (!add_move(conf, x, y, x + step * dx, y + step * dy, successors, num_successors)) break;
            }
        }
    }
}

void generate_rook_moves(struct config *conf, int x, int y, struct config *successors, int *num_successors) {
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if ((dx == 0 || dy == 0) && (dx != dy)) {
                for (int step = 1; step < BOARD_SIZE; step++) {
                    if (!add_move(conf, x, y, x + step * dx, y + step * dy, successors, num_successors)) break;
                }
            }
        }
    }
}

void generate_queen_moves(struct config *conf, int x, int y, struct config *successors, int *num_successors) {
    generate_bishop_moves(conf, x, y, successors, num_successors);
    generate_rook_moves(conf, x, y, successors, num_successors);
}

// Generate Pawn moves
void generate_pawn_moves(struct config *conf, int x, int y, struct config *successors, int *num_successors) {
    if (conf->board[x][y] == 'P') { // White pawn
        // Forward move
        if (x > 0 && conf->board[x - 1][y] == ' ') {
            add_move(conf, x, y, x - 1, y, successors, num_successors);

            // Double move from starting position
            if (x == 6 && conf->board[x - 2][y] == ' ') {
                add_move(conf, x, y, x - 2, y, successors, num_successors);
            }
        }

        // Captures
        if (x > 0 && y > 0 && conf->board[x - 1][y - 1] >= 'a' && conf->board[x - 1][y - 1] <= 'z') {
            add_move(conf, x, y, x - 1, y - 1, successors, num_successors);
        }
        if (x > 0 && y < BOARD_SIZE - 1 && conf->board[x - 1][y + 1] >= 'a' && conf->board[x - 1][y + 1] <= 'z') {
            add_move(conf, x, y, x - 1, y + 1, successors, num_successors);
        }

        // Promotion
        if (x - 1 == 0) {
            struct config promoted_conf = *conf;
            promoted_conf.board[x - 1][y] = 'Q'; // Promote to Queen (default)
            promoted_conf.board[x][y] = ' ';
            promoted_conf.turn *= -1;
            successors[*num_successors] = promoted_conf;
            (*num_successors)++;
        }
    } else if (conf->board[x][y] == 'p') { // Black pawn
        // Forward move
        if (x < BOARD_SIZE - 1 && conf->board[x + 1][y] == ' ') {
            add_move(conf, x, y, x + 1, y, successors, num_successors);

            // Double move from starting position
            if (x == 1 && conf->board[x + 2][y] == ' ') {
                add_move(conf, x, y, x + 2, y, successors, num_successors);
            }
        }

        // Captures
        if (x < BOARD_SIZE - 1 && y > 0 && conf->board[x + 1][y - 1] >= 'A' && conf->board[x + 1][y - 1] <= 'Z') {
            add_move(conf, x, y, x + 1, y - 1, successors, num_successors);
        }
        if (x < BOARD_SIZE - 1 && y < BOARD_SIZE - 1 && conf->board[x + 1][y + 1] >= 'A' && conf->board[x + 1][y + 1] <= 'Z') {
            add_move(conf, x, y, x + 1, y + 1, successors, num_successors);
        }

        // Promotion
        if (x + 1 == BOARD_SIZE - 1) {
            struct config promoted_conf = *conf;
            promoted_conf.board[x + 1][y] = 'q'; // Promote to Queen (default)
            promoted_conf.board[x][y] = ' ';
            promoted_conf.turn *= -1;
            successors[*num_successors] = promoted_conf;
            (*num_successors)++;
        }
    }
}

//Pawn Validation
int validate_pawn_move(struct config *board, int from_x, int from_y, int to_x, int to_y) {
    char piece = board->board[from_x][from_y];
    int direction = (piece == 'P') ? -1 : 1; // White pawns move up (-1), Black pawns move down (+1)

    // Check if moving forward
    if (from_y == to_y) {
        if (to_x == from_x + direction && board->board[to_x][to_y] == ' ') {
            // Single step forward
            return 1;
        }
        if ((piece == 'P' && from_x == 6) || (piece == 'p' && from_x == 1)) {
            // Double step forward from starting position
            if (to_x == from_x + 2 * direction &&
                board->board[from_x + direction][from_y] == ' ' &&
                board->board[to_x][to_y] == ' ') {
                return 1;
            }
        }
    }

    // Check for diagonal capture
    if (abs(to_y - from_y) == 1 && to_x == from_x + direction) {
        char target = board->board[to_x][to_y];
        if ((piece == 'P' && target >= 'a' && target <= 'z') || // White pawn captures Black
            (piece == 'p' && target >= 'A' && target <= 'Z')) { // Black pawn captures White
            return 1;
        }
    }

    // If none of the conditions match, the move is invalid
    return 0;
}

// King Validation
int validate_king_move(struct config *board, int from_x, int from_y, int to_x, int to_y) {
    if (abs(to_x - from_x) <= 1 && abs(to_y - from_y) <= 1) {
        char target = board->board[to_x][to_y];
        if (target == ' ' || 
           (board->turn == MAX && target >= 'a' && target <= 'z') ||
           (board->turn == MIN && target >= 'A' && target <= 'Z')) {
            return 1; // Valid king move
        }
    }
    return 0; // Invalid king move
}

//Queen Validation
int validate_queen_move(struct config *board, int from_x, int from_y, int to_x, int to_y) {
    if (validate_rook_move(board, from_x, from_y, to_x, to_y) ||
        validate_bishop_move(board, from_x, from_y, to_x, to_y)) {
        return 1; // Queen can move as a rook or bishop
    }
    return 0;
}

//Rook Validation
int validate_rook_move(struct config *board, int from_x, int from_y, int to_x, int to_y) {
    if (from_x == to_x || from_y == to_y) {
        int dx = (to_x - from_x) ? (to_x - from_x) / abs(to_x - from_x) : 0;
        int dy = (to_y - from_y) ? (to_y - from_y) / abs(to_y - from_y) : 0;
        for (int step = 1; step < abs(to_x - from_x + to_y - from_y); step++) {
            if (board->board[from_x + step * dx][from_y + step * dy] != ' ') {
                return 0; // Path is blocked
            }
        }
        char target = board->board[to_x][to_y];
        if (target == ' ' || 
           (board->turn == MAX && target >= 'a' && target <= 'z') ||
           (board->turn == MIN && target >= 'A' && target <= 'Z')) {
            return 1; // Valid rook move
        }
    }
    return 0; // Invalid rook move
}

//Bishop Validation
int validate_bishop_move(struct config *board, int from_x, int from_y, int to_x, int to_y) {
    if (abs(to_x - from_x) == abs(to_y - from_y)) {
        int dx = (to_x - from_x) / abs(to_x - from_x);
        int dy = (to_y - from_y) / abs(to_y - from_y);
        for (int step = 1; step < abs(to_x - from_x); step++) {
            if (board->board[from_x + step * dx][from_y + step * dy] != ' ') {
                return 0; // Path is blocked
            }
        }
        char target = board->board[to_x][to_y];
        if (target == ' ' || 
           (board->turn == MAX && target >= 'a' && target <= 'z') ||
           (board->turn == MIN && target >= 'A' && target <= 'Z')) {
            return 1; // Valid bishop move
        }
    }
    return 0; // Invalid bishop move
}

//Knight Validation
int validate_knight_move(struct config *board, int from_x, int from_y, int to_x, int to_y) {
    if ((abs(to_x - from_x) == 2 && abs(to_y - from_y) == 1) ||
        (abs(to_x - from_x) == 1 && abs(to_y - from_y) == 2)) {
        char target = board->board[to_x][to_y];
        if (target == ' ' || 
           (board->turn == MAX && target >= 'a' && target <= 'z') ||
           (board->turn == MIN && target >= 'A' && target <= 'Z')) {
            return 1; // Valid knight move
        }
    }
    return 0; // Invalid knight move
}


//Player Move
int player_move(struct config *board, char *from, char *to) {
    int from_x = 8 - (from[1] - '0');
    int from_y = from[0] - 'a';
    int to_x = 8 - (to[1] - '0');
    int to_y = to[0] - 'a';

    // Check if the input is out of bounds
    if (from_x < 0 || from_x >= BOARD_SIZE || to_x < 0 || to_x >= BOARD_SIZE ||
        from_y < 0 || from_y >= BOARD_SIZE || to_y < 0 || to_y >= BOARD_SIZE) {
        printf("Invalid move: Out of bounds. Please try again.\n");
        return 0; // Invalid move
    }

    char piece = board->board[from_x][from_y];
    int is_valid = 0; // Tracks if the move is valid

    // Validate based on the piece type
    switch (piece) {
        case 'P': case 'p':
            is_valid = validate_pawn_move(board, from_x, from_y, to_x, to_y);
            break;
        case 'K': case 'k':
            is_valid = validate_king_move(board, from_x, from_y, to_x, to_y);
            break;
        case 'Q': case 'q':
            is_valid = validate_queen_move(board, from_x, from_y, to_x, to_y);
            break;
        case 'R': case 'r':
            is_valid = validate_rook_move(board, from_x, from_y, to_x, to_y);
            break;
        case 'B': case 'b':
            is_valid = validate_bishop_move(board, from_x, from_y, to_x, to_y);
            break;
        case 'N': case 'n':
            is_valid = validate_knight_move(board, from_x, from_y, to_x, to_y);
            break;
        default:
            printf("Invalid move: No piece at the source square. Please try again.\n");
            return 0;
    }

    if (is_valid) {
        // Make the move
        board->board[to_x][to_y] = piece;
        board->board[from_x][from_y] = ' ';
        board->turn *= -1; // Switch turn
        return 1; // Move executed successfully
    } else {
        printf("Invalid move for piece %c from (%d, %d) to (%d, %d). Please try again.\n",
               piece, from_x, from_y, to_x, to_y);
        return 0;
    }
}


int add_move(struct config *conf, int from_x, int from_y, int to_x, int to_y, struct config *successors, int *num_successors) {
    if (to_x >= 0 && to_x < BOARD_SIZE && to_y >= 0 && to_y < BOARD_SIZE) {
        char target = conf->board[to_x][to_y];
        if ((conf->turn == MAX && (target == ' ' || (target >= 'a' && target <= 'z'))) || 
            (conf->turn == MIN && (target == ' ' || (target >= 'A' && target <= 'Z')))) {
            struct config new_conf = *conf;
            new_conf.board[to_x][to_y] = conf->board[from_x][from_y];
            new_conf.board[from_x][from_y] = ' ';
            new_conf.turn *= -1; // Switch turn
            successors[*num_successors] = new_conf;
            (*num_successors)++;
            return 1; // Move successfully added
        }
    }
    return 0; // Invalid move
}

// Check if game is over
int is_game_over(struct config *board) {
    int white_king = 0, black_king = 0;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board->board[i][j] == 'K') white_king = 1;
            if (board->board[i][j] == 'k') black_king = 1;
        }
    }

    return !(white_king && black_king); // Game over if a king is missing
}
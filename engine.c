#include "engine.h"
#include "board.h"
#include <stdio.h>
#include <omp.h>
#include <limits.h>

// Engine chooses the best move
void engine_move(struct config *board) {
    struct config successors[100];
    int num_successors = 0;

    // Generate all possible moves
    generate_successors(board, successors, &num_successors);

    // Check if there are no successors
    if (num_successors == 0) {
        printf("No moves available. Game over.\n");
        return;
    }

    // Initialize best move
    int best_score = INT_MIN;
    struct config best_move = successors[0];

    // Determine depth dynamically based on game complexity
    int depth = (num_successors > 30) ? 2 : 3; // Shallower depth for higher branching factor

    // Parallelized minimax evaluation
    #pragma omp parallel for
    for (int i = 0; i < num_successors; i++) {
        int score = minimax(successors[i], depth, MIN, INT_MIN, INT_MAX);
        #pragma omp critical
        {
            if (score > best_score) {
                best_score = score;
                best_move = successors[i];
            }
        }
    }

    // Apply the best move
    *board = best_move;
}

// Minimax with Alpha-Beta Pruning
int minimax(struct config board, int depth, int maximizingPlayer, int alpha, int beta) {
    if (depth == 0 || is_game_over(&board)) {
        return evaluate_board(&board);
    }

    struct config successors[100];
    int num_successors = 0;
    generate_successors(&board, successors, &num_successors);

    if (maximizingPlayer == MAX) {
        int max_eval = INT_MIN;
        int prune = 0; // Flag for pruning
        #pragma omp parallel for reduction(max:max_eval)
        for (int i = 0; i < num_successors; i++) {
            if (prune) continue; // Skip iterations if pruning is triggered
            int eval = minimax(successors[i], depth - 1, MIN, alpha, beta);
            #pragma omp critical
            {
                max_eval = (eval > max_eval) ? eval : max_eval;
                alpha = (alpha > eval) ? alpha : eval;
                if (beta <= alpha) prune = 1; // Trigger pruning
            }
        }
        return max_eval;
    } else {
        int min_eval = INT_MAX;
        int prune = 0; // Flag for pruning
        #pragma omp parallel for reduction(min:min_eval)
        for (int i = 0; i < num_successors; i++) {
            if (prune) continue; // Skip iterations if pruning is triggered
            int eval = minimax(successors[i], depth - 1, MAX, alpha, beta);
            #pragma omp critical
            {
                min_eval = (eval < min_eval) ? eval : min_eval;
                beta = (beta < eval) ? beta : eval;
                if (beta <= alpha) prune = 1; // Trigger pruning
            }
        }
        return min_eval;
    }

}

// Enhanced board evaluation function
int evaluate_board(struct config *board) {
    int score = 0;

    // Assign points for each piece and incorporate positional value
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            switch (board->board[i][j]) {
                case 'P': score += 1 + (6 - i); break; // Reward advancing pawns
                case 'R': score += 5; break;
                case 'N': score += 3 + ((i > 2 && i < 6 && j > 2 && j < 6) ? 1 : 0); break; // Centralized knights
                case 'B': score += 3; break;
                case 'Q': score += 9; break;
                case 'K': score += 100 - (board->turn == MAX ? i : (7 - i)); break; // Encourage king safety
                case 'p': score -= 1 + (i - 1); break; // Penalize advancing pawns
                case 'r': score -= 5; break;
                case 'n': score -= 3 + ((i > 2 && i < 6 && j > 2 && j < 6) ? 1 : 0); break; // Centralized knights
                case 'b': score -= 3; break;
                case 'q': score -= 9; break;
                case 'k': score -= 100 - (board->turn == MIN ? (7 - i) : i); break; // Encourage king safety
            }
        }
    }

    return score;
}

#ifndef ENGINE_H
#define ENGINE_H

#include "board.h"

// Engine Functions
void engine_move(struct config *board); // Selects the best move for the computer

// Minimax Algorithm with Alpha-Beta Pruning
int minimax(struct config board, int depth, int maximizingPlayer, int alpha, int beta);

// Board Evaluation
int evaluate_board(struct config *board);

#endif

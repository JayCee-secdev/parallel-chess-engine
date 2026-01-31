#include "board.h"
#include "engine.h"
#include <stdio.h>
#include <string.h>

int main() {
    struct config board;
    init_board(&board);

    printf("Welcome to Optimized Chess Engine!\n");
    printf("You are White. Enter moves in standard chess notation (e.g., e2 e4).\n");
    printf("Type 'quit' to exit the game or 'recommend' to see move suggestions.\n");

    while (1) {
        print_board(&board);

        // Player's turn
        char from[10], to[10];
        int valid_move = 0;
        while (!valid_move) {
            printf("\nEnter your move (e.g., e2 e4, 'recommend' for suggestions, or 'quit' to exit): ");
            scanf("%s", from);

            // Handle special commands
            if (strcmp(from, "quit") == 0) {
                printf("\nGoodbye! Thanks for playing.\n");
                return 0; // Exit the program immediately
            }
            if (strcmp(from, "recommend") == 0) {
                printf("\nGenerating move recommendations...\n");
                recommend_moves(&board);
                continue; // Go back to prompting for the move
            }

            // If not a command, read the target square
            scanf("%s", to);

            // Handle quit command on second input
            if (strcmp(to, "quit") == 0) {
                printf("\nGoodbye! Thanks for playing.\n");
                return 0; // Exit the program immediately
            }
            if (strcmp(to, "recommend") == 0) {
                printf("\n'recommend' can only be used as the first input.\n");
                continue; // Go back to prompting for the move
            }

            // Validate and execute player's move
            valid_move = player_move(&board, from, to);
            if (!valid_move) {
                printf("\nInvalid move! Please try again.\n");
            }
        }

        // Check if the player has won
        if (is_game_over(&board)) {
            printf("\nCongratulations! You win. Game over.\n");
            break;
        }

        // Engine's turn
        printf("\nEngine is thinking...\n");
        engine_move(&board);

        // Provide the current board evaluation for feedback
        int eval_score = evaluate_board(&board);
        printf("\nEngine's move completed. Current board evaluation: %d\n", eval_score);

        // Check if the engine has won
        if (is_game_over(&board)) {
            printf("\nThe engine wins. Better luck next time! Game over.\n");
            break;
        }
    }

    return 0;
}

# Parallel Chess Engine
A high-performance chess engine developed in C that utilizes OpenMP to optimize move evaluation and decision-making through parallel computing. This was the final project for my CDA4125: Concepts of Parallel & Distributed Processing course.

## 📌 Project Overview
Traditional chess engines face combinatorial complexity, making exhaustive game-tree searches computationally expensive as depth increases. This project implements a parallelized Minimax algorithm with Alpha-Beta pruning to distribute workloads across multiple CPU cores, significantly reducing computation time while maintaining move quality.

## 🚀 Key Features
- Parallelized Decision Making: Leverages OpenMP for thread-level parallelism during the evaluation of board successors.
- Advanced Move Generation: Modular functions to generate legal moves for all pieces, including complex rules like castling, pawn promotions, and capture mechanics.
- Strategic Evaluation: A multi-factored evaluation function that weights material value, positional control (centralization), and king safety.
- Interactive Interface: A command-line interface supporting standard chess notation for human-vs-computer play.

## 🛠️ Technical Implementation
### Core Algorithms
- Minimax Algorithm: Recursively explores the game tree to find optimal moves for the engine while assuming optimal play from the opponent.
- Alpha-Beta Pruning: Enhances search efficiency by eliminating branches that cannot influence the final decision, drastically reducing the search space.

### Parallelization Strategy:
- Thread-Level Parallelism: Uses #pragma omp parallel for to evaluate different branches of the game tree concurrently.
- Dynamic Load Balancing: Implements dynamic scheduling to manage varying numbers of successors between moves, ensuring optimal CPU utilization.
- Critical Sections: Ensures safe updates to the "best move" shared variables to prevent race conditions.

## 📂 File Structure
1. main.c: The entry point managing the game loop and user interaction.
2. board.c / board.h: Implements core game mechanics, board initialization, and piece-specific move generation.
3. engine.h: Declares functions for board evaluation and engine decision-making logic.

## ⚙️ Requirements & Usage
Compiler: GCC or any C compiler supporting OpenMP.

Build:
```
gcc -fopenmp main.c board.c -o chess_engine
```
Run:
```
./chess_engine
```

## 📈 Performance & Scalability
The project demonstrates substantial speedup on multi-core architectures, particularly in move-intensive mid-game phases. Key challenges addressed include minimizing synchronization overhead and managing dynamic workloads to prevent diminishing returns at high search depths.

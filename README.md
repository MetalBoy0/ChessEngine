# Chess AI

## Inspiration
I created this chess AI mainly because of Sebastian Lague's video on Chess AIs and to also serve as a way to learn C++. Watching his approach to developing a chess engine and learning about Stockfish, whose source code is available on GitHub, motivated me to start my own project. 

## Features
This chess AI is equipped with several features that allow it to perform efficiently and effectively:

- **Bitboards:** Efficiently represent the chessboard and its pieces using bit manipulation.
- **Magic Numbers:** Optimize move generation for sliding pieces like bishops and rooks.
- **Alpha-Beta Pruning:** Reduce the number of positions evaluated in the minimax algorithm.
- **Quiescence Search:** Extend the search at "quiet" positions to avoid horizon effects.
- **Transposition Table:** Cache previously evaluated positions to prevent redundant calculations.
- **Iterative Deepening:** Enhance move ordering and allow for a flexible depth of search.
- **Move Sorting:** Prioritize promising moves to maximize the effectiveness of alpha-beta pruning.
- **Piece Square Tables:** Evaluate positional advantages for each piece based on its position on the board.

### Performance
On my machine, this AI calculates around **2.6 million moves/positions per second**.

## Areas for Improvement
While the chess AI is already capable, there are several enhancements that could further improve its functionality and performance:

1. **Code Readability:**
   - Add more comments and documentation to make the codebase easier to understand and maintain.

2. **Performance Optimization:**
   - Incorporate **multi-threading** to speed up searches by leveraging modern CPUs.
   - Implement additional pruning techniques and depth reductions/extensions to refine the search tree.

3. **Opening Strategy:**
   - Integrate an **opening book** to play established strong moves in the early game.

4. **Evaluation Function:**
   - Improve board evaluation by considering factors like:
     - The quality of trades and their impact on material balance.
     - **King safety**, such as pawn shielding and threats.
     - Dynamic **piece square tables** that adjust based on the game phase (opening, midgame, or endgame).

By working on these improvements, I aim to make the AI stronger, faster, and easier to work with.


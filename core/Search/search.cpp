#include <cassert>
#include <chrono>

#include "evaluate.h"
#include "moveOrder.h"
#include "search.h"
#include "../uci.h"
#include "../movegen/movegen.h"
#include "../representation/board.h"
#include "../representation/move.h"

using namespace std;

// clang-format off
#define POSINF           1000000
#define NEGINF          -1000000
#define FUTILITY_MARGIN  200
#define RAZORING_MARGIN  500
// clang-format on

unsigned long long startTime;
float maxTimeMS = 0;
bool stopSearch = false;

TranspositionTable *tt = new TranspositionTable(pow(2, 20));

void clearTTSearch() { tt->clear(); }

int futilityMargin(int depth)
{
    return FUTILITY_MARGIN * depth;
}

ostream &operator<<(ostream &os, const MoveStack &moveVal)
{
    for (int i = 0; i < moveVal.count; i++)
    {
        os << moveToString(moveVal.moves[i]) << " ";
    }
    return os;
}

// Update the path of the stack with the move and the child stack
void update_path(MoveStack *stack, Move move, const MoveStack *childStack)
{
    stack->moves[0] = move;
    for (int i = 0; i < childStack->count; i++)
    {
        stack->moves[i + 1] = childStack->moves[i];
    }
    stack->count = childStack->count + 1;
}

struct SearchDiagnostics
{
    unsigned long long nodes;
    unsigned long long qNodes;
    unsigned long long time;
    unsigned long long cutoffs;
    unsigned int futilityCutoffs;
    unsigned int razoringCutoffs;
    unsigned int transpositionCuttoffs;
};

struct MoveVal
{
    Move move;
    float value;
};

SearchDiagnostics diagnostics;
MoveVal bestMove;
Move startMove;

bool IsMate(int score) { return abs(score) > POSINF - MAX_DEPTH; }

float qsearch(Board *board, int ply, float alpha, float beta)
{
    diagnostics.qNodes++;
    float eval = evaluate(board);
    diagnostics.nodes++;
    if (eval >= beta)
    {
        diagnostics.cutoffs++;
        return beta;
    }
    if (eval > alpha)
    {
        alpha = eval;
    }

    MoveList moveList;
    generateMoves(board, moveList, true);
    sortMoves(&moveList, 0, board, true);
    if (moveList.count == 0)
    {
        return eval;
    }
    for (int i = 0; i < moveList.count; i++)
    {

        board->makeMove(moveList.moves[i]);
        float value = -qsearch(board, ply + 1, -beta, -alpha);
        board->undoMove();

        if (value >= beta)
        {
            return beta;
        }
        if (value > alpha)
        {
            alpha = value;
        }
    }
    return alpha;
}

int search(Board *board, unsigned int depth, int ply, float alpha,
           float beta, MoveStack *stack)
{

    // check for time
    if (!stopSearch && chrono::duration_cast<chrono::nanoseconds>(
                           chrono::system_clock::now().time_since_epoch())
                                   .count() -
                               startTime >
                           maxTimeMS * 1000000)
    {
        stopSearch = true;
    }

    if (stopSearch)
    {
        return 0;
    }

    // If we are at a leaf node, we call qsearch
    if (depth == 0)
    {
        return qsearch(board, ply, alpha, beta); // qsearch(board, ply, alpha, beta);
    }

    // If we aren't at a root node, we check if we can do a cutoff
    if (ply > 0)
    {
        alpha = max(alpha, NEGINF + ply);
        beta = min(beta, POSINF - ply);
        if (alpha >= beta)
        {

            diagnostics.cutoffs++;
            return alpha;
        }
    }

    // Transposition Table Lookup
    int ttVal = tt->probe(board->zobristKey, depth, alpha, beta);

    if (ttVal != tt->failed)
    {
        if (ply == 0)
        {
            bestMove.move = tt->getMove(board->zobristKey);
            bestMove.value = ttVal;
        }

        diagnostics.transpositionCuttoffs++;

        return ttVal;
    }

    // Initialize node stuff
    diagnostics.nodes++;
    float eval = evaluate(board);

    // Futility pruning
    if (depth < 4 && !board->inCheck && eval - futilityMargin(depth) >= beta &&
        !IsMate(eval))
    {

        diagnostics.cutoffs++;
        diagnostics.futilityCutoffs++;

        return eval - futilityMargin(depth);
    }

    // Razoring
    if (depth == 1 && !board->inCheck && eval + RAZORING_MARGIN <= alpha &&
        !IsMate(eval))
    {

        diagnostics.cutoffs++;

        return qsearch(board, ply, alpha, beta);
    }

    MoveList moveList;
    generateMoves(board, moveList);

    if (moveList.count == 0)
    {
        if (board->inCheck)
        {
            return NEGINF + ply; // Checkmate
        }
        else
        {
            return 0; // Stalemate
        }
    }
    sortMoves(&moveList, startMove, board);
    TranspositionTable::EvalType evalType = TranspositionTable::Upper;

    Move bestMoveCurrent = 0; // Best move for current node

    // Iterate through all moves
    for (int i = 0; i < moveList.count; i++)
    {
        Move move = moveList.moves[i];
        board->makeMove(move);
        float value = -search(board, depth - 1, ply + 1, -beta, -alpha, (stack + 1));
        board->undoMove();

        if (stopSearch)
        {
            return 0;
        }

        if (value >= beta)
        {
            tt->store(board->zobristKey, depth, value, move,
                      TranspositionTable::Lower);

            return beta;
        }
        if (value > alpha)
        {
            alpha = value;
            evalType = TranspositionTable::Exact;
            if (ply == 0)
            {
                bestMove.move = move;
                bestMove.value = value;
                cout << "info string current best " << moveToString(bestMove.move) << "\n";
            }
            bestMoveCurrent = move;
            update_path(stack, move, (stack + 1));
        }
    }
    tt->store(board->zobristKey, depth, alpha, bestMoveCurrent, evalType);

    return alpha;
}

void startIterativeDeepening(Board *board, unsigned int maxDepth,
                             int maxTime = 0, int maxNodes = 0)
{
    diagnostics.nodes = 0;
    diagnostics.qNodes = 0;
    diagnostics.time = 0;
    diagnostics.cutoffs = 0;
    diagnostics.futilityCutoffs = 0;
    diagnostics.transpositionCuttoffs = 0;

    startMove = 0;

    MoveVal prevBestMove = bestMove;

    for (int i = 1; i <= maxDepth; i++)
    {
        unsigned long long startDepthTime =
            chrono::duration_cast<chrono::milliseconds>(
                chrono::system_clock::now().time_since_epoch())
                .count();
        diagnostics.time = startDepthTime;

        startMove = bestMove.move;
        prevBestMove = bestMove;

        bestMove.move = 0;
        bestMove.value = NEGINF;

        MoveStack stack[MAX_DEPTH];
        MoveStack *bestStack = stack;

        for (int d = 0; d < MAX_DEPTH; d++)
        {
            stack[d].count = 0;
            for (int j = 0; j < MAX_DEPTH; j++)
            {
                stack[d].moves[j] = 0;
            }
        }

        search(board, i, 0, NEGINF, POSINF, bestStack);

        if (stopSearch)
        {
            cout << "info string stopping search, using search results from depth " << i - 1 << "\n";
            bestMove = prevBestMove;
            break;
        }

        unsigned long long currentTime =
            chrono::duration_cast<chrono::milliseconds>(
                chrono::system_clock::now().time_since_epoch())
                .count();

        int used = (float)(tt->used) / (float)(tt->size) * 1000;

        cout << "info depth "   << i
             << " score cp "    << bestMove.value
             << " nodes "       << diagnostics.nodes 
             << " nps "         << (int)((double)diagnostics.nodes / (double)(currentTime - startDepthTime + 1) * 1000)
             << " hashfull "    << used 
             << " pv "          << moveToString(bestMove.move) 
             << " time "        << currentTime - startDepthTime << "\n";

        diagnostics.nodes = 0;
        diagnostics.qNodes = 0;
        diagnostics.time = 0;
        diagnostics.cutoffs = 0;
        diagnostics.futilityCutoffs = 0;
        diagnostics.transpositionCuttoffs = 0;

        if (diagnostics.nodes > maxNodes && maxNodes != 0)
        {
            break;
        }
        if (i == maxDepth && maxDepth != 0)
        {
            break;
        }
    }
}

Move startSearch(Board *board, unsigned int depth, int maxTime, int maxNodes,
                 int wtime, int btime)
{
    // Initialize search
    startTime = chrono::duration_cast<chrono::nanoseconds>(
                    chrono::system_clock::now().time_since_epoch())
                    .count();
    maxTimeMS = maxTime == 0 ? 10000000 : maxTime - 50; // Subtract 50ms to be safe
    stopSearch = false;

    bestMove.value = -100000;
    bestMove.move = 0;
    startIterativeDeepening(board, depth, maxTime, maxNodes);
    return bestMove.move;
}

unsigned long long perft(Board *board, const unsigned int depth)
{
    MoveList moveList;
    generateMoves(board, moveList);
    if (depth == 1U)
    {
        return moveList.count;
    }
    else if (depth == 0U)
    {
        return 1;
    }

    unsigned long long nodes = 0;
    for (int i = 0; i < moveList.count; i++)
    {
        Move move = moveList.moves[i];
        board->makeMove(move);
        nodes += perft(board, depth - 1);
        board->undoMove();
    }
    return nodes;
}

unsigned long long startPerft(Board board, unsigned int depth)
{
    unsigned long long nodes = 0;
    MoveList moveList;
    generateMoves(&board, moveList);
    for (int i = 0; i < moveList.count; i++)
    {
        Move move = moveList.moves[i];
        board.makeMove(move);
        unsigned long long mNode = perft(&board, depth - 1);
        board.undoMove();
        cout << moveToString(move) << ": " << mNode << "\n";
        nodes += mNode;
    }
    return nodes;
}
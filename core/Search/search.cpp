#include "../../uci.h"
#include "../MoveGeneration/movegen.h"
#include "../Representation/board.h"
#include "../Representation/move.h"
#include "evaluate.h"
#include "moveOrder.h"
#include "search.h"
#include <cassert>
#include <chrono>

using namespace std;

#define MAX_DEPTH 64
#define POSINF 1000000
#define NEGINF -1000000

TranspositionTable *tt = new TranspositionTable(pow(2, 20));

float min(float a, float b) { return a < b ? a : b; }

float max(float a, float b) { return a > b ? a : b; }

void clearTTSearch() { tt->clear(); }

struct searchDiagnostics
{
    unsigned int nodes;
    unsigned int qNodes;
    unsigned long long time;
    unsigned int cutoffs;
    unsigned int transpositionCuttoffs;
};

searchDiagnostics diagnostics;

struct MoveVal
{
    Move move;
    float value;
};

MoveVal bestMove;
Move startMove;
MoveList path;

bool IsMate(int score) { return abs(score) > POSINF - MAX_DEPTH; }

float qsearch(Board *board, int ply, float alpha, float beta)
{
    diagnostics.qNodes++;
    int eval = evaluate(board);
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
    generateMoves(board, &moveList, true);
    sortMoves(&moveList, 0, board, true);
    if (moveList.count == 0)
    {
        return eval;
    }
    for (int i = 0; i < moveList.count; i++)
    {
        // unsigned long long oldZ = board->zobristKey;
        // if (oldZ == 18446744072653600672ULL)
        //{
        //     cout << "bad";
        // }
        int score = board->score;
        board->makeMove(moveList.moves[i]);
        float value = -qsearch(board, ply + 1, -beta, -alpha);
        board->undoMove();

        // if (oldZ != board->zobristKey)
        //{
        //     cout << "Bad";
        // }
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

float search(Board *board, unsigned int depth, int ply, float alpha,
             float beta)
{

    if (depth == 0)
    {
        return qsearch(board, ply, alpha,
                       beta); // qsearch(board, ply, alpha, beta);
    }
    if (ply > 0)
    {
        alpha = max(alpha, -POSINF + ply);
        beta = min(beta, POSINF - ply);
        if (alpha >= beta)
        {
            diagnostics.cutoffs++;
            return alpha;
        }
    }

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

    // diagnostics.nodes++;

    MoveList moveList;
    generateMoves(board, &moveList);

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

    Move bestMoveCurrent = 0;

    for (int i = 0; i < moveList.count; i++)
    {
        Move move = moveList.moves[i];
        // unsigned long long oldZ = board->zobristKey;
        int score = board->score;

        board->makeMove(move);
        float value = -search(board, depth - 1, ply + 1, -beta, -alpha);
        board->undoMove();
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
                cout << "currmove " << moveToString(bestMove.move) << "\n";
            }
            path.moves[ply] = move;
            bestMoveCurrent = move;
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
    diagnostics.transpositionCuttoffs = 0;
    startMove = 0;
    unsigned long long startTime =
        chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now().time_since_epoch())
            .count();
    for (int i = 1; i <= maxDepth; i++)
    {
        unsigned long long startDepthTime =
            chrono::duration_cast<chrono::milliseconds>(
                chrono::system_clock::now().time_since_epoch())
                .count();
        diagnostics.time = startDepthTime;
        bestMove.move = 0;
        bestMove.value = -100000;
        cout << board->score << "\n";

        search(board, i, 0, NEGINF, POSINF);

        unsigned long long currentTime =
            chrono::duration_cast<chrono::milliseconds>(
                chrono::system_clock::now().time_since_epoch())
                .count();
        int used = (float)(tt->used) / (float)(tt->size) * 1000;
        cout << "info depth " << i << " score cp " << bestMove.value << " nodes "
             << diagnostics.nodes << " nps "
             << diagnostics.nodes * 1000 / (currentTime - startDepthTime + 1)
             << " hashfull " << used << " pv " << diagnostics.transpositionCuttoffs
             << "\n";
        startMove = bestMove.move;

        if ((currentTime - startTime) > maxTime && maxTime != 0)
        {
            break;
        }
        if (diagnostics.nodes > maxNodes && maxNodes != 0)
        {
            break;
        }
        if (i == maxDepth && maxDepth != 0)
        {
            break;
        }

        diagnostics.nodes = 0;
        diagnostics.qNodes = 0;
        diagnostics.time = 0;
        diagnostics.cutoffs = 0;
        diagnostics.transpositionCuttoffs = 0;
    }
}

Move startSearch(Board *board, unsigned int depth, int maxTime, int maxNodes,
                 int wtime, int btime)
{

    if (wtime != 0 && btime != 0)
    {
        maxTime = (wtime + btime) / 4;
    }
    bestMove.value = -100000;
    bestMove.move = 0;
    startIterativeDeepening(board, depth, maxTime, maxNodes);
    path.count = depth;
    return bestMove.move;
}

unsigned int perft(Board *board, unsigned int depth)
{
    MoveList moveList;
    generateMoves(board, &moveList);
    if (depth == 1)
    {
        return moveList.count;
    }
    else if (depth == 0)
    {
        return 1;
    }

    unsigned int nodes = 0;
    for (int i = 0; i < moveList.count; i++)
    {
        Move move = moveList.moves[i];
        unsigned long long oldColorBB = board->colorBB[Pieces::Color::Black];
        int score = board->score;
        board->makeMove(move);
        nodes += perft(board, depth - 1);
        board->undoMove();

        if (oldColorBB != board->colorBB[Pieces::Color::Black])
        {
            cout << moveToString(move) << " " << depth;
        }
    }
    return nodes;
}

unsigned int startPerft(Board board, unsigned int depth)
{
    unsigned int nodes = 0;
    MoveList moveList;
    generateMoves(&board, &moveList);
    for (int i = 0; i < moveList.count; i++)
    {
        Move move = moveList.moves[i];
        board.makeMove(move);
        int mNode = perft(&board, depth - 1);
        board.undoMove();
        cout << moveToString(move) << ": " << mNode << "\n";
        nodes += mNode;
    }
    return nodes;
}
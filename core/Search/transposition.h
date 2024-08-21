#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

#include "../representation/move.h"
#include "../representation/board.h"

class TranspositionTable
{
public:
    int failed = INT32_MIN;
    enum EvalType : uint8_t
    {
        Exact,
        Lower,
        Upper,
        NOTINIT
    };
    struct Entry
    {
        unsigned long long zobrist;
        char depth;
        int value;
        EvalType evalType;
        Move bestMove;
    };

private:
    Entry *table;
    

public:
    TranspositionTable(int size);
    ~TranspositionTable();

    int used = 0;
    int size;

    void store(unsigned long long zobrist, int depth, int value, Move bestMove, EvalType evalType);
    int probe(unsigned long long zobrist, int depth, int alpha, int beta);
    Move getMove(unsigned long long zobrist);
    void clear();

    int getSize() { return size; }
    int getUsed();
};

#endif
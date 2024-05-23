#include "transposition.h"

void TranspositionTable::store(unsigned long long zobrist, int depth, int value, Move bestMove, EvalType evalType)
{
    int index = zobrist % size;
    Entry entry = table[index];
    if (entry.evalType == NOTINIT)
    {
        used++;
    }
    entry.zobrist = zobrist;
    entry.depth = depth;
    entry.value = value;
    entry.bestMove = bestMove;
    entry.evalType = evalType;
    table[index] = entry;
}

Move TranspositionTable::getMove(unsigned long long zobrist)
{
    int index = zobrist % size;
    Entry entry = table[index];
    if (entry.zobrist == zobrist)
    {
        return entry.bestMove;
    }
    return failed;
}

int TranspositionTable::probe(unsigned long long zobrist, int depth, int alpha, int beta)
{
    unsigned int index = zobrist % size;
    Entry entry = table[index];
    if (entry.zobrist == zobrist && entry.depth >= depth)
    {
        int score = entry.value;

        if (entry.evalType == Exact)
        {
            return score;
        }

        if (entry.evalType == Upper && score <= alpha)
        {
            return score;
        }

        if (entry.evalType == Lower && score >= beta)
        {
            return score;
        }
    }
    return failed;
}

void TranspositionTable::clear()
{
    for (int i = 0; i < size; i++)
    {
        table[i].zobrist = 0;
    }
}

TranspositionTable::TranspositionTable(int size)
{
    int entrySize = sizeof Entry();
    this->size = (int)(size / entrySize);
    this->table = new Entry[this->size];
    for (int i = 0; i < this->size; i++)
    {
        table[i].evalType = NOTINIT;
        table[i].zobrist = 0;
        table[i].depth = 0;
        table[i].value = 0;
        table[i].bestMove = 0;
    }
    for (int i = 0; i < this->size; i++)
    {
        Entry t = table[i];
        if (t.evalType != NOTINIT)
        {
            cout << i << " " << t.evalType << "\n";
        }
    }
}

TranspositionTable::~TranspositionTable()
{
    delete[] table;
}

int TranspositionTable::getUsed()
{
    return used;
}

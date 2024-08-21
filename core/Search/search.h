#ifndef SEARCH_H
#define SEARCH_H

#include <cmath>

#include "../representation/board.h"
#include "transposition.h"



struct MoveStack {
    Move moves[256];
    int count;
};

extern ostream &operator<<(ostream &os, const MoveStack &stack);
inline void operator+=(MoveStack &a, Move b) {
    a.moves[a.count++] = b;
}


extern unsigned long long startPerft(Board board, unsigned int depth);
extern Move startSearch(Board *board, unsigned int depth, int maxTime, int maxNodes, int wtime, int btime);
extern void clearTTSearch();

#endif
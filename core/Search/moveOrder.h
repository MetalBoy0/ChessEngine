#ifndef MOVEORDER_H
#define MOVEORDER_H

#include "../representation/board.h"
#include "../movegen/movegen.h"

extern void sortMoves(MoveList *moves, Move prevMove, Board *board, bool onlyCaptures = false);

#endif
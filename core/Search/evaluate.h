#ifndef EVALUATE_H
#define EVALUATE_H

#include "../representation/board.h"

constexpr int pieceValues[7] = { 0, 100, 300, 320, 500, 900, 100000 };

extern float evaluate(Board* board);


#endif
#ifndef DIRECTION_H
#define DIRECTION_H
#include <cstdint>
#include <cstdlib>

enum Direction : int8_t
{
    NULLDIR = 0,
    N = 8,
    S = -8,
    E = 1,
    W = -1,
    NE = 9,
    NW = 7,
    SE = -7,
    SW = -9,
    NN = 16, // For pawns
    SS = -16,
    NNE = 17, // For knights
    NEE = 10,
    SEE = -6,
    SSE = -15,
    SSW = -17,
    SWW = -10,
    NWW = 6,
    NNW = 15
};

const Direction bishopDirections[4] = {NE, SE, NW, SW};
const Direction rookDirections[4] = {N, S, E, W};
const Direction queenDirections[8] = {N, S, E, W, NE, SE, NW, SW};

extern Direction directions[64][64];
extern uint8_t distToEdge[64][8];
extern void initDirections();

const unsigned char _tmp[19] = {7, 1, 6, 0, 0, 0, 0, 0, 3, 0, 2, 0, 0, 0, 0, 0, 5, 0, 4};

namespace
{
    const unsigned char *_dirIndex = _tmp + 9;
    
    inline constexpr unsigned char getDirIndex(const Direction dir)
    {
        return (_dirIndex)[dir];
    }

    Direction getDirectionBetween(int from, int to)
    {
        return directions[from][to];
    }

    Direction invertDirection(Direction dir)
    {
        return (Direction)(-dir);
    }

    bool onEdge(int to)
    {
        return (!distToEdge[to][0] || !distToEdge[to][1] || !distToEdge[to][2] ||
                !distToEdge[to][3] || !distToEdge[to][4] || !distToEdge[to][5] ||
                !distToEdge[to][6] || !distToEdge[to][7]);
    }

}

#endif
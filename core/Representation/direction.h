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

constexpr Direction bishopDirections[4] = {NE, NW, SE, SW};
constexpr Direction rookDirections[4] = {N, S, E, W};
constexpr Direction queenDirections[8] = {N, S, E, W, NE, NW, SE, SW};

extern Direction directions[64][64];
extern uint8_t distToEdge[64][9];
extern void initDirections();
// SE  S  SW                W     E                 NW N  SE
constexpr unsigned char _tmp[19] = {8, 2, 7, 0, 0, 0, 0, 0, 4, 0, 3, 0, 0, 0, 0, 0, 6, 1, 5};

namespace
{
    const unsigned char *_dirIndex = _tmp + 9;

    constexpr unsigned char getDirIndex(const Direction dir)
    {
        return (_dirIndex)[dir];
    }

    inline Direction getDirectionBetween(const int from, const int to)
    {
        return directions[from][to];
    }

    constexpr Direction operator~(const Direction dir)
    {
        return (Direction)(-dir);
    }

    inline bool onEdge(const int to)
    {
        return (!distToEdge[to][0] || !distToEdge[to][1] || !distToEdge[to][2] ||
                !distToEdge[to][3] || !distToEdge[to][4] || !distToEdge[to][5] ||
                !distToEdge[to][6] || !distToEdge[to][7]);
    }

    constexpr bool isDiagonal(const Direction dir)
    {
        return dir == NE || dir == SE || dir == NW || dir == SW;
    }

    constexpr bool isStraight(const Direction dir)
    {
        return dir == N || dir == S || dir == E || dir == W;
    }
}

#endif
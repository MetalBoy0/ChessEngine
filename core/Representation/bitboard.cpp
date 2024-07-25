#include <iostream>
#include "bitboard.h"
#include <cassert>
using namespace std;

const Bitboard rankMasks[8] = {
    0x00000000000000FF,
    0x000000000000FF00,
    0x0000000000FF0000,
    0x00000000FF000000,
    0x000000FF00000000,
    0x0000FF0000000000,
    0x00FF000000000000,
    0xFF00000000000000};

const Bitboard fileMasks[8] = {
    0x0101010101010101,
    0x0202020202020202,
    0x0404040404040404,
    0x0808080808080808,
    0x1010101010101010,
    0x2020202020202020,
    0x4040404040404040,
    0x8080808080808080};

const Bitboard middleMask = fullBB ^ (fileMasks[0] | fileMasks[7]);

const Bitboard shortCastle[9] = {
    0b11ull << 61,
    0, 0, 0, 0, 0, 0, 0,
    0b1100000};

const Bitboard longCastle[10] = {
    0b111ull << 57,
    0b110ull << 57, 0, 0, 0, 0, 0, 0,
    0b1110ull, 0b1100ull};

Bitboard knightMoves[64];

Bitboard kingMoves[64];

Bitboard dirToBB[8][64];

Bitboard bitboardRays[64][64];

Bitboard setupBitboardRay(int from, int to);

void initBBs()
{
    // Knights
    for (int i = 0; i < 64; i++)
    {
        Bitboard bb = 0ULL;
        int rank = i / 8;
        int file = i % 8;
        if (rank < 6 && file < 7)
        {
            setBit(&bb, i + 17);
        }
        if (rank < 6 && file > 0)
        {
            setBit(&bb, i + 15);
        }
        if (rank < 7 && file < 6)
        {
            setBit(&bb, i + 10);
        }
        if (rank < 7 && file > 1)
        {
            setBit(&bb, i + 6);
        }
        if (rank > 0 && file < 6)
        {
            setBit(&bb, i - 6);
        }
        if (rank > 0 && file > 1)
        {
            setBit(&bb, i - 10);
        }
        if (rank > 1 && file < 7)
        {
            setBit(&bb, i - 15);
        }
        if (rank > 1 && file > 0)
        {
            setBit(&bb, i - 17);
        }
        knightMoves[i] = bb;
    }
    // Kings
    for (int i = 0; i < 64; i++)
    {
        Bitboard bb = 0ULL;
        int rank = i / 8;
        int file = i % 8;
        if (rank < 7)
        {
            setBit(&bb, i + 8);
        }
        if (rank > 0)
        {
            setBit(&bb, i - 8);
        }
        if (file < 7)
        {
            setBit(&bb, i + 1);
        }
        if (file > 0)
        {
            setBit(&bb, i - 1);
        }
        if (rank < 7 && file < 7)
        {
            setBit(&bb, i + 9);
        }
        if (rank < 7 && file > 0)
        {
            setBit(&bb, i + 7);
        }
        if (rank > 0 && file < 7)
        {
            setBit(&bb, i - 7);
        }
        if (rank > 0 && file > 0)
        {
            setBit(&bb, i - 9);
        }
        kingMoves[i] = bb;
    }
    // Setup dirToBB
    Direction queenDirectionsOpp[8] = {S, N, W, E, SW, NW, SE, NE};
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            Bitboard bb = 0ULL;
            dirToBB[j][i] = sendRay(&bb, queenDirections[j], i) | sendRay(&bb, queenDirectionsOpp[j], i);
        }
    }
    for (int i = 0; i < 64; i++)
    {
        for (int y = 0; y < 64; y++)
        {
            bitboardRays[i][y] = setupBitboardRay(i, y);
        }
    }
}

void printBitboard(Bitboard *bb)
{
    for (int i = 0; i < 64; i++)
    {
        if (i % 8 == 0)
        {
            cout << endl;
        }
        cout << getBit(bb, i) << " ";
    }
    cout << endl;
}

// A bitboard of all the squares in the ray from the square in the direction
Bitboard bitboardRay(Direction dir, int square)
{
    Bitboard bb = 0ULL;
    int i = square;
    while (i >= 0 && i < 64)
    {
        setBit(&bb, i);
        i += dir;
    }
    return bb;
}

// A bitboard of all the squares in the ray from the square to the square (includes starting square but not ending square   )
Bitboard setupBitboardRay(int from, int to)
{
    Bitboard bb = 0ULL;
    Direction dir = getDirectionBetween(from, to);
    // assert (from != to);
    if (dir == NULLDIR)
    {
        return 0ULL;
    }
    int i = from;
    while (i != to)
    {
        setBit(&bb, i);
        i += dir;
    }
    return bb;
}

Bitboard bitboardRay(int from, int to) { return bitboardRays[from][to]; }

Bitboard sendRay(const Bitboard* bb, const Direction dir, const int square) 
{

    Bitboard obb = 0;
    const unsigned char dirIndex = getDirIndex(dir);

    if (distToEdge[square][dirIndex] == 0) {
        return obb;
    }

    int current = square + dir;
    while (distToEdge[current][dirIndex] != 0 && !(((*bb) >> current) & 1)) {
        obb |= (1ULL << current);
        current += dir;
    }
    obb |= (1ULL << current);
    return obb;
}
#include <iostream>
#include <cassert>

#include "bitboard.h"
#include "magicBB.h"

using namespace std;



Bitboard knightMoves[64];

Bitboard kingMoves[64];

Bitboard dirToBB[9][64];

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
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            Bitboard bb = 0ULL;
            dirToBB[getDirIndex(queenDirections[j])][i] = sendRayPre(&bb, queenDirections[j], i);
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

/* Sends a ray in the direction from the square
However it's slow and should not be used after the magic bitboards are initialized
*/
Bitboard sendRayPre(const Bitboard *bb, const Direction dir, const int square)
{

    Bitboard obb = 0;
    unsigned char dirIndex = getDirIndex(dir);

    if (distToEdge[square][dirIndex] == 0)
    {
        return obb;
    }

    int current = square + dir;
    while (distToEdge[current][dirIndex] != 0 && !(((*bb) >> current) & 1))
    {
        obb |= (1ULL << current);
        current += dir;
    }
    obb |= (1ULL << current);
    return obb;
}

// Sends a ray in the direction from the square (after the magic bitboards are initialized)
Bitboard sendRay(const Bitboard *bb, const Direction dir, const int square)
{
    unsigned char dirIndex = getDirIndex(dir);

    if (distToEdge[square][dirIndex] == 0)
    {
        return 0ULL; // No squares in the direction
    }

    Bitboard dirBB = dirToBB[dirIndex][square];

    // get magic bitboard for the direction and square
    if (isStraight(dir))
    {
        return rookMagics[square].table[getKey(*bb & rookMasks[square], &rookMagics[square])] & dirBB;
    }
    else if (isDiagonal(dir))
    {
        return bishopMagics[square].table[getKey(*bb & bishopMasks[square], &bishopMagics[square])] & dirBB;
    }
    else
    {
        assert(false); // Invalid direction
    }
}


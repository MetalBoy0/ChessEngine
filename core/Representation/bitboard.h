#ifndef BITBOARD_H
#define BITBOARD_H
#include "direction.h"
#include "piece.h"
typedef unsigned long long Bitboard;

// Some helpful bitboard masks

const Bitboard emptyBB = 0ull;
const Bitboard fullBB = ~emptyBB;

// castling masks
extern const Bitboard shortCastle[9];
extern const Bitboard longCastle[10];

// File masks
extern const Bitboard fileMasks[8];
extern const Bitboard middleMask;

// Rank masks
extern const Bitboard rankMasks[8];

// Knight moves
extern Bitboard knightMoves[64];

// King moves
extern Bitboard kingMoves[64];

// Direction BBs
extern Bitboard diagonalsBB[64];
extern Bitboard straightsBB[64];
extern Bitboard canSeeBB[64];
extern Bitboard dirToBB[8][64];

extern void printBitboard(Bitboard *bb);
extern Bitboard bitboardRay(Direction dir, int square);
extern Bitboard bitboardRay(int from, int to);
extern Bitboard sendRay(const Bitboard *bb, const Direction dir, const int square);
extern void initBBs();
namespace
{

    Bitboard getBitboardFromSquare(int square)
    {
        return 1ull << square;
    }

    // Returns a bool if there is a 1 at the specified square else 0
    inline constexpr bool getBit(const Bitboard *bitboard, const int square)
    {
        return *bitboard >> square & 1ULL;
    }

    inline constexpr bool getBit(const Bitboard bitboard, const int square)
    {
        return bitboard >> square & 1ULL;
    }

    inline constexpr void setBit(Bitboard *bitboard, const int square)
    {
        *bitboard |= 1ULL << square;
    }

    void clearBit(Bitboard *bitboard, const int square)
    {
        *bitboard &= ~(1ULL << square);
    }

    void toggleBit(Bitboard *bitboard, const int square)
    {
        *bitboard ^= 1ULL << square;
    }

    inline int getLSB(const Bitboard *bb)
    {
        return __builtin_ctzll(*bb);
    }

    inline int popLSB(Bitboard *bb)
    {
        const int index = getLSB(bb);
        *bb &= *bb - 1; // Not depend on the previous line
        return index;
    }

    inline int popCount(const Bitboard *bb)
    {
        return __builtin_popcountll(*bb);
    }

    inline int popCount(Bitboard bb)
    {
        return __builtin_popcountll(bb);
    }

    template <Direction dir>
    Bitboard shift(Bitboard *bb)
    {
        return dir > 0 ? *bb << dir : *bb >> -dir;
    }

    Bitboard shift(Bitboard *bb, Direction dir, int amount)
    {
        return dir > 0 ? *bb << (dir * amount) : *bb >> (-dir * amount);
    }

    template <Piece>
    Bitboard getAttackBB(int s);
    template <Piece>
    Bitboard getAttackBB(int s, Bitboard *squares);

    template <>
    Bitboard getAttackBB<Pieces::Pawn>(int s)
    {
    }

    template <>
    inline Bitboard getAttackBB<Pieces::Knight>(const int s)
    {
        return knightMoves[s];
    }

    template <>
    Bitboard getAttackBB<Pieces::Bishop>(int s, Bitboard *squares)
    {
        return  sendRay(squares, NW, s) | sendRay(squares, SW, s) | sendRay(squares, NE, s) | sendRay(squares, SE, s);
    }

    template <>
    Bitboard getAttackBB<Pieces::Rook>(int s, Bitboard *squares)
    {
        return sendRay(squares, N, s) | sendRay(squares, S, s) | sendRay(squares, E, s) | sendRay(squares, W, s);
    }

    template <>
    Bitboard getAttackBB<Pieces::Queen>(int s, Bitboard *squares)
    {
    }

    template <>
    Bitboard getAttackBB<Pieces::King>(int s)
    {
        return kingMoves[s];
    }
}
#endif
#ifndef BITBOARD_H
#define BITBOARD_H

#include "direction.h"
#include "piece.h"

typedef unsigned long long Bitboard;

// Some helpful bitboard masks


constexpr Bitboard emptyBB = 0ull;
constexpr Bitboard fullBB = ~emptyBB;



// Rank masks
constexpr Bitboard rankMasks[8] = {
    0x00000000000000FF,
    0x000000000000FF00,
    0x0000000000FF0000,
    0x00000000FF000000,
    0x000000FF00000000,
    0x0000FF0000000000,
    0x00FF000000000000,
    0xFF00000000000000};

// File masks
constexpr Bitboard fileMasks[8] = {
    0x0101010101010101,
    0x0202020202020202,
    0x0404040404040404,
    0x0808080808080808,
    0x1010101010101010,
    0x2020202020202020,
    0x4040404040404040,
    0x8080808080808080};



// castling masks
constexpr Bitboard shortCastle[9] = {
    0b11ull << 61,
    0, 0, 0, 0, 0, 0, 0,
    0b1100000};

constexpr Bitboard longCastle[10] = {
    0b111ull << 57,
    0b110ull << 57, 0, 0, 0, 0, 0, 0,
    0b1110ull, 0b1100ull};


constexpr Bitboard middleMask = fullBB ^ (fileMasks[0] | fileMasks[7]);

// Knight moves
extern Bitboard knightMoves[64];

// King moves
extern Bitboard kingMoves[64];

// Direction BBs
extern Bitboard diagonalsBB[64];
extern Bitboard straightsBB[64];
extern Bitboard canSeeBB[64];
extern Bitboard dirToBB[9][64];

extern void printBitboard(Bitboard *bb);
extern Bitboard bitboardRay(Direction dir, int square);
extern Bitboard bitboardRay(int from, int to);
extern Bitboard sendRay(const Bitboard *bb, const Direction dir, const int square);
extern Bitboard sendRayPre(const Bitboard *bb, const Direction dir, const int square);
extern void initBBs();



#ifdef _MSC_VER

    inline int getLSB(const Bitboard *bb)
    {
        // Get the least significant bit using _BitScanForward64
        unsigned long index;
        _BitScanForward64(&index, *bb);
        return index;
    }

    inline int popCount(const Bitboard *bb)
    {
        return __popcnt64(*bb);
    }

    inline int popCount(Bitboard bb)
    {
        return __popcnt64(bb);
    }

#else

    constexpr inline int getLSB(const Bitboard *bb)
    {
        return __builtin_ctzll(*bb);
    }

    constexpr inline int popCount(const Bitboard *bb)
    {
        return __builtin_popcountll(*bb);
    }

    constexpr inline int popCount(Bitboard bb)
    {
        return __builtin_popcountll(bb);
    }

#endif


constexpr Bitboard getBitboardFromSquare(uint8_t square)
{
    return 1ull << square;
}

// Returns a bool if there is a 1 at the specified square else 0
inline bool getBit(const Bitboard *bitboard, const int square)
{
    return *bitboard >> square & 1ULL;
}

inline bool getBit(const Bitboard bitboard, const int square)
{
    return bitboard >> square & 1ULL;
}

inline void setBit(Bitboard *bitboard, const int square)
{
    *bitboard |= 1ULL << square;
}

inline void clearBit(Bitboard *bitboard, const int square)
{
    *bitboard &= ~(1ULL << square);
}

inline void toggleBit(Bitboard *bitboard, const int square)
{
    *bitboard ^= 1ULL << square;
}

inline int popLSB(Bitboard *bb)
{
    int index = getLSB(bb);
    *bb &= *bb - 1; // Not depend on the previous line
    return index;
}

template <Direction dir>
inline Bitboard shift(Bitboard *bb)
{
    return dir > 0 ? *bb << dir : *bb >> -dir;
}

inline Bitboard shift(Bitboard *bb, Direction dir, int amount)
{
    return dir > 0 ? *bb << (dir * amount) : *bb >> (-dir * amount);
}



#endif
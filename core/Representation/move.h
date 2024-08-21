#ifndef MOVE_H
#define MOVE_H

#include <cstdint>

#include "piece.h"

// Moves will be a 24 bit unsigned integer
typedef uint32_t Move;

// Move format:
// 0000 0000 0000 000000 000000
// Capt ECCP Prom   To    From
// ECCP: En Passant, Castle, Capture, Promotion



constexpr bool isCapture(Move move)
{
    return (move >> 18 & 1);
}

constexpr bool isCastle(Move move)
{
    return ((move >> 19) & 1) != 0;
}


constexpr bool isPromotion(Move move)
{
    return (move >> 17 & 1) != 0;
}

constexpr Piece getPromotion(Move move)
{
    return (Piece)((move >> 12) & 0x7);
}

constexpr int getTo(Move move)
{
    return (move >> 6) & 0x3F;
}

constexpr int getFrom(Move move)
{
    return move & 0x3F;
}

constexpr Piece getCapturedPiece(Move move)
{
    return (move >> 20) & 0xF;
}


#endif
#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "../representation/move.h"
#include "../representation/board.h"
#include "../representation/magicBB.h"

struct MoveList
{
    Move moves[256];
    int count;
};

extern std::ostream &operator<<(std::ostream &os, const MoveList &moveList); // print a movelist

inline void operator+=(MoveList &moveList, Move move) { moveList.moves[moveList.count++] = move; }

// main move generation function
extern void generateMoves(Board *board, MoveList &moveList, bool onlyCaptures = false);

template <Pieces::PieceType>
extern Bitboard getAttackBB(const int s);
template <Pieces::PieceType>
extern Bitboard getAttackBB(int s, Bitboard *squares);

template <>
inline Bitboard getAttackBB<Pieces::Knight>(const int s)
{
    return knightMoves[s];
}

template <>
inline Bitboard getAttackBB<Pieces::Bishop>(int s, Bitboard *squares)
{
    return bishopMagics[s].table[getKey(*squares & bishopMasks[s], &bishopMagics[s])];
}

template <>
inline Bitboard getAttackBB<Pieces::Rook>(int s, Bitboard *squares)
{
    return rookMagics[s].table[getKey(*squares & rookMasks[s], &rookMagics[s])];
}

template <>
inline Bitboard getAttackBB<Pieces::Queen>(int s, Bitboard *squares)
{
    return getAttackBB<Pieces::Bishop>(s, squares) | getAttackBB<Pieces::Rook>(s, squares);
}

template <>
inline Bitboard getAttackBB<Pieces::King>(int s)
{
    return kingMoves[s];
}

#endif
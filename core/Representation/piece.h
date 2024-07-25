#ifndef PIECE_H
#define PIECE_H

#include <cstdint>

using namespace std;

// Pieces will be a 4 bit unsigned integer

typedef uint8_t Piece;

// Piece format:
// 0 000
// S Typ
// S is the side (0 for white, 1 for black)
// Typ is the type of piece (0 for empty, 1 for pawn, 2 for knight, 3 for bishop, 4 for rook, 5 for queen, 6 for king)

namespace Pieces
{
    enum PieceType : uint8_t
    {
        Empty,
        Pawn,
        Knight,
        Bishop,
        Rook,
        Queen,
        King
    };

    enum Color : uint8_t
    {
        White = 0,
        None = 1,
        Black = 8
    };



    inline constexpr PieceType getType(const Piece piece)
    {
        return static_cast<PieceType>(piece & 7);
    }

    // Returns true if the piece is black
    constexpr bool isBlack(Piece piece)
    {
        return (piece & 8) == 8;
    }

    // Returns true if the piece is empty
    constexpr bool isEmpty(Piece piece)
    {
        return piece == Empty; // Might need to change this later on, this relies on the fact that an empty piece is white and not black
    }

    constexpr Color getColor(Piece piece)
    {
        return isEmpty(piece) ? None : static_cast<Color>(piece & 8);
    }

    // Returns true if the piece is white
    constexpr bool isPieceWhite(Piece piece)
    {
        return ((piece & 8) == 0) && (!isEmpty(piece));
    }

    constexpr bool isWhite(Piece piece)
    {
        return ((piece & 8) == 0);
    }

    // Returns true if the piece is a pawn
    constexpr bool isPawn(Piece piece)
    {
        return (piece & 7) == Pawn;
    }

    // Returns true if the piece is a knight
    constexpr bool isKnight(Piece piece)
    {
        return (piece & 7) == Knight;
    }

    // Returns true if the piece is a bishop
    constexpr bool isBishop(Piece piece)
    {
        return (piece & 7) == Bishop;
    }

    // Returns true if the piece is a rook
    constexpr bool isRook(Piece piece)
    {
        return (piece & 7) == Rook;
    }

    // Returns true if the piece is a queen
    constexpr bool isQueen(Piece piece)
    {
        return (piece & 7) == Queen;
    }

    // Returns true if the piece is a king
    constexpr bool isKing(Piece piece)
    {
        return (piece & 7) == King;
    }

    // Returns true if the piece is of the specified type
    template <PieceType type>
    constexpr bool isPiece(Piece piece)
    {
        return (piece & 7) != type;
    }

    // Returns true if the piece is a sliding piece (bishop, rook, or queen)
    constexpr bool isSliding(Piece piece)
    {
        return isBishop(piece) || isRook(piece) || isQueen(piece);
    }

    // Returns true if the piece is a non-sliding piece (pawn, knight, or king)
    constexpr bool isNonSliding(Piece piece)
    {
        return isPawn(piece) || isKnight(piece) || isKing(piece);
    }

    constexpr Color invertColor(Color color)
    {
        return static_cast<Color>(color ^ 8);
    }

    // Turns a char into the corresponding piece
    Piece charToPiece(char c);
    char pieceToChar(Piece piece);
}

#endif
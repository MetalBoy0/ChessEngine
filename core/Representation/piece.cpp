#include <iostream>

#include "piece.h"


using namespace std;

Piece Pieces::charToPiece(char c)
{
    Color side = isupper(c) ? White : Black; // Gets the color of the piece, if char is Uppercase it is white, othewise it is black
    PieceType type;
    switch (tolower(c))
    {
    case 'p':
        type = Pawn;
        break;
    case 'n':
        type = Knight;
        break;
    case 'b':
        type = Bishop;
        break;
    case 'r':
        type = Rook;
        break;
    case 'q':
        type = Queen;
        break;
    case 'k':
        type = King;
        break;
    default:
        type = Empty; // This shouldn't happen
        break;
    }
    Piece piece = side | type; // You can create a piece by Or'ing the side and the type
    return piece;
}

char Pieces::pieceToChar(Piece piece)
{
    char c;
    switch (getType(piece))
    {
    case Pawn:
        c = 'p';
        break;
    case Knight:
        c = 'n';
        break;
    case Bishop:
        c = 'b';
        break;
    case Rook:
        c = 'r';
        break;
    case Queen:
        c = 'q';
        break;
    case King:
        c = 'k';
        break;
    default:
        c = ' ';
        break;
    }
    if (isPieceWhite(piece))
    {
        c = toupper(c);
    }
    return c;
}
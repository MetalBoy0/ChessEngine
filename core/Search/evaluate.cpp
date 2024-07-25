#include "evaluate.h"
#include "../Representation/board.h"
#include "../MoveGeneration/movegen.h"

float pieceTableValue(const int table[], int square, bool isWhite)
{
    return table[(isWhite ? indexToRank(square) * 8 + indexToFile(square) : square)];
}

float getTablePieceValues(pieceList pieces, bool isWhite, const int table[])
{
    float score = 0;
    for (int i = 0; i < pieces.count; i++)
    {
        score += pieceTableValue(table, pieces.pieces[i], isWhite);
    }
    return score;
}

float evaluate(Board *board)
{
    float score = 0;

    // Add up piece values
    score += pieceValues[Pieces::Pawn] * popCount(board->pieceBB[Pieces::Pawn] & board->colorBB[Pieces::White]);
    score -= pieceValues[Pieces::Pawn] * popCount(board->pieceBB[Pieces::Pawn] & board->colorBB[Pieces::Black]);
    score += pieceValues[Pieces::Knight] * popCount(board->pieceBB[Pieces::Knight] & board->colorBB[Pieces::White]);
    score -= pieceValues[Pieces::Knight] * popCount(board->pieceBB[Pieces::Knight] & board->colorBB[Pieces::Black]);
    score += pieceValues[Pieces::Bishop] * popCount(board->pieceBB[Pieces::Bishop] & board->colorBB[Pieces::White]);
    score -= pieceValues[Pieces::Bishop] * popCount(board->pieceBB[Pieces::Bishop] & board->colorBB[Pieces::Black]);
    score += pieceValues[Pieces::Rook] * popCount(board->pieceBB[Pieces::Rook] & board->colorBB[Pieces::White]);
    score -= pieceValues[Pieces::Rook] * popCount(board->pieceBB[Pieces::Rook] & board->colorBB[Pieces::Black]);
    score += pieceValues[Pieces::Queen] * popCount(board->pieceBB[Pieces::Queen] & board->colorBB[Pieces::White]);
    score -= pieceValues[Pieces::Queen] * popCount(board->pieceBB[Pieces::Queen] & board->colorBB[Pieces::Black]);
    score += pieceValues[Pieces::King] * popCount(board->pieceBB[Pieces::King] & board->colorBB[Pieces::White]);
    score -= pieceValues[Pieces::King] * popCount(board->pieceBB[Pieces::King] & board->colorBB[Pieces::Black]);

    // Piece Tables
    score += board->score;
    return score * (board->isWhite * 2 - 1);
}
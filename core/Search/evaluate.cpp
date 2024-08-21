#include "evaluate.h"
#include "pieceSquareTable.h"
#include "../representation/bitboard.h"
#include "../representation/board.h"
#include "../movegen/movegen.h"

int allPieces(Board* board)
{
    int score = 0;
    constexpr Pieces::PieceType pieces[] = { Pieces::Pawn, Pieces::Knight, Pieces::Bishop, Pieces::Rook, Pieces::Queen, Pieces::King };

    for (Pieces::PieceType piece : pieces)
    {
        if (board->pieceBB[piece])
        {
            Bitboard wpieces = board->pieceBB[piece] & board->colorBB[Pieces::White];
            Bitboard bpieces = board->pieceBB[piece] & board->colorBB[Pieces::Black];

			score += popCount(wpieces) * pieceValues[piece];
			score -= popCount(bpieces) * pieceValues[piece];

            while (wpieces)
            {
                int square = popLSB(&wpieces);
                score += PSQT::psq[piece][square];
            }

            while (bpieces)
            {
                int square = popLSB(&bpieces);
                score -= flipTable(PSQT::psq[piece])[square];
            }
        }
    }
    return score;
}

float evaluate(Board* board)
{
    float score = 0;

    // Piece Tables
    score += allPieces(board);

    // Control of the board
    score += popCount(board->attackedBB[Pieces::White]) - popCount(board->attackedBB[Pieces::Black]);

    return score * (board->isWhite ? 1 : -1);
}
#include "movegen.h"
#include "../Representation/piece.h"

int captures = 0;
int checks = 0;

bool distIsMoreThanOne(int from, int to)
{
    return (abs(indexToRank(from) - indexToRank(to)) > 1) || (abs(indexToFile(from) - indexToFile(to)) > 1);
}

void generateCheckBB(Board *board)
{
    board->checkingBB = -1;
    Bitboard king = board->pieceBB[Pieces::King] & board->colorBB[board->sideToMove];
    int kingIndex = getLSB(&king);

    board->checkers.count = 0;

    if (board->inCheck)
    {
        board->checkingBB = 0;

        // Check for knight checks

        Bitboard knights = board->pieceBB[Pieces::Knight] & board->colorBB[board->otherSide];
        Bitboard canCheck = knightMoves[kingIndex] & knights;
        appendBB(&board->checkers, canCheck);

        // Check for pawn checks
        if (board->sideToMove == Pieces::Color::White)
        {
            Bitboard pawns = board->pieceBB[Pieces::Pawn] & board->colorBB[board->otherSide];
            Bitboard canCheck = ((shift<SE>(&king) | shift<SW>(&king)) & kingMoves[kingIndex]) & pawns;
            appendBB(&board->checkers, canCheck);
        }
        else
        {
            Bitboard pawns = board->pieceBB[Pieces::Pawn] & board->colorBB[board->otherSide];
            Bitboard canCheck = ((shift<NE>(&king) | shift<NW>(&king)) & kingMoves[kingIndex]) & pawns;
            appendBB(&board->checkers, canCheck);
        }

        // Check for sliding checks
        Bitboard sliding = (board->pieceBB[Pieces::Rook] | board->pieceBB[Pieces::Queen] | board->pieceBB[Pieces::Bishop]) & board->colorBB[board->otherSide];

        while (sliding)
        {
            int piece = popLSB(&sliding);

            // Check if the rook can see the king
            Direction dir = getDirectionBetween(piece, kingIndex);
            if (dir == NULLDIR || dir == NNW || dir == NNE || dir == SSW || dir == SSE || dir == NEE || dir == SEE || dir == NWW || dir == SWW)
            {
                continue;
            }
            if (Pieces::getType(board->board[piece]) == Pieces::Bishop && (dir == N || dir == S || dir == E || dir == W))
            {
                continue;
            }
            if (Pieces::getType(board->board[piece]) == Pieces::Rook && (dir == NE || dir == NW || dir == SE || dir == SW))
            {
                continue;
            }
            // Check if there is a piece in between the rook and the king
            Bitboard between = sendRay(&board->allPiecesBB, dir, piece);
            if (between & king)
            {
                board->checkers.index[board->checkers.count] = piece;
                board->checkers.count++;
                if (board->checkers.count > 1)
                {
                    break;
                }
            }
        }
        if (board->checkers.count == 1 && Pieces::getType(board->board[board->checkers.index[0]]) != Pieces::Knight)
        {
            board->checkingBB = sendRay(&board->allPiecesBB, getDirectionBetween(kingIndex, board->checkers.index[0]), kingIndex);
        }
        else if (board->checkers.count > 1)
        {
            // Multiple checkers so nothing can block
            board->checkingBB = 0;
        }
        else if (board->checkers.count == 1)
        {
            // Checker is a knight
            board->checkingBB = getBitboardFromSquare(board->checkers.index[0]);
        }
    }
}

bool legalKnightMove(int from, int to)
{
    int rankF = indexToRank(from);
    int fileF = indexToFile(from);
    int rankT = indexToRank(to);
    int fileT = indexToFile(to);
    if (abs(rankF - rankT) == 2 && abs(fileF - fileT) == 1)
    {
        return true;
    }
    if (abs(rankF - rankT) == 1 && abs(fileF - fileT) == 2)
    {
        return true;
    }
    return false;
}

void appendMove(MoveList *moveList, Move move)
{
    moveList->moves[moveList->count] = move;
    moveList->count++;
}

void generatePawnMoves(Board *board, MoveList *MoveList, bool onlyCaptures)
{
    // Shift the pawn BBs to get the moves
    Bitboard kingBB = board->pieceBB[Pieces::King] & board->colorBB[board->sideToMove];
    int kingIndex = getLSB(&kingBB);
    Bitboard pawns = board->pieceBB[Pieces::Pawn] & board->colorBB[board->sideToMove];
    Bitboard moves = 0;
    Bitboard captureE = 0;
    Bitboard captureW = 0;

    if (board->sideToMove == Pieces::Color::White)
    {
        moves |= shift<S>(&pawns);
    }
    else
    {
        moves |= shift<N>(&pawns);
    }

    if (board->sideToMove == Pieces::Color::White)
    {
        captureE |= shift<SE>(&pawns);
        captureW |= shift<SW>(&pawns);
    }
    else
    {
        captureE |= shift<NE>(&pawns);
        captureW |= shift<NW>(&pawns);
    }

    moves &= board->pieceBB[0];
    // Add en passant to each of these
    Bitboard mask = (board->colorBB[board->otherSide]);
    Bitboard enPassantCheck = 0;
    if (board->enPassantSquare != -1)
    {
        mask |= (1ULL << board->enPassantSquare);
        // Check if the en passant will remove the king from check
        // If king is checked by pawn but not double checked
        if (board->checkers.count == 1 && Pieces::getType(board->board[board->checkers.index[0]]) == Pieces::Pawn)
        {
            int up = board->sideToMove == Pieces::Color::White ? 8 : -8;
            if (board->checkers.index[0] - up == board->enPassantSquare)
            {
                enPassantCheck = getBitboardFromSquare(board->enPassantSquare);
            }
        }
    }
    captureE &= mask;
    captureW &= mask;

    // In check
    captureE &= board->checkingBB | enPassantCheck;
    captureW &= board->checkingBB | enPassantCheck;

    // Decode BBs into moves
    if (!onlyCaptures)
    {
        while (moves)
        {

            int to = popLSB(&moves);
            int from = to + (board->sideToMove == Pieces::Color::White ? 8 : -8);

            Direction pin = board->isPinned(from);
            if (pin != NULLDIR)
            {
                if (!(pin == N || pin == S))
                {
                    continue;
                }
            }
            if ((getBitboardFromSquare(to) & board->checkingBB))
            {
                if (indexToRank(to) == 0 || indexToRank(to) == 7)
                {
                    appendMove(MoveList, board->getMove(from, to, Pieces::Queen));
                    appendMove(MoveList, board->getMove(from, to, Pieces::Rook));
                    appendMove(MoveList, board->getMove(from, to, Pieces::Bishop));
                    appendMove(MoveList, board->getMove(from, to, Pieces::Knight));
                }
                else
                {
                    appendMove(MoveList, board->getMove(from, to));
                }
            }
            // Double pawn push

            if ((indexToRank(from) == 1 && board->isWhite) || (indexToRank(from) == 6 && !board->isWhite))
            {
                int to2 = to - (board->sideToMove == Pieces::Color::White ? 8 : -8);
                if (!(getBitboardFromSquare(to2) & board->checkingBB))
                {
                    continue;
                }
                if (board->board[to2] == Pieces::Empty)
                {
                    appendMove(MoveList, board->getMove(from, to2));
                }
            }
        }
    }
    while (captureE)
    {
        int to = popLSB(&captureE);
        int from = to - (board->sideToMove == Pieces::Color::White ? SE : NE);

        if (distToEdge[from][getDirIndex(E)] == 0)
        {
            continue;
        }
        Direction pin = board->isPinned(from);

        if (pin != NULLDIR)
        {
            if (getDirectionBetween(from, to) != pin && getDirectionBetween(to, from) != pin)
            {
                continue;
            }
        }
        // Handle en passant by removing the pieces from the bitboard and checking if the king is under attack

        if (to == board->enPassantSquare && indexToRank(kingIndex) == indexToRank(from))
        {

            clearBit(&board->pieceBB[Pieces::Pawn], from);
            clearBit(&board->colorBB[board->sideToMove], from);
            clearBit(&board->allPiecesBB, from);
            board->board[from] = Pieces::Empty;

            int capturedPawn = to - (board->sideToMove == Pieces::Color::White ? -8 : 8);
            clearBit(&board->pieceBB[Pieces::Pawn], capturedPawn);
            clearBit(&board->colorBB[board->otherSide], capturedPawn);
            clearBit(&board->allPiecesBB, capturedPawn);
            board->board[capturedPawn] = Pieces::Empty;

            // send ray from king to the pawn direction
            Direction dir = getDirectionBetween(kingIndex, from);
            Bitboard ray = sendRay(&board->allPiecesBB, dir, kingIndex);

            // Check if the ray hits an enemy sliding piece
            Bitboard sliding = (board->pieceBB[Pieces::Rook] | board->pieceBB[Pieces::Queen]) & board->colorBB[board->otherSide];

            // Undo the en passant
            setBit(&board->pieceBB[Pieces::Pawn], from);
            setBit(&board->colorBB[board->sideToMove], from);
            setBit(&board->allPiecesBB, from);
            board->board[from] = Pieces::Pawn | board->sideToMove;

            setBit(&board->pieceBB[Pieces::Pawn], capturedPawn);
            setBit(&board->colorBB[board->otherSide], capturedPawn);
            setBit(&board->allPiecesBB, capturedPawn);
            board->board[capturedPawn] = Pieces::Pawn | board->otherSide;

            if (sliding & ray)
            {
                continue;
            }
        }

        if (indexToRank(to) == 0 || indexToRank(to) == 7)
        {
            appendMove(MoveList, board->getMove(from, to, Pieces::Queen));
            appendMove(MoveList, board->getMove(from, to, Pieces::Rook));
            appendMove(MoveList, board->getMove(from, to, Pieces::Bishop));
            appendMove(MoveList, board->getMove(from, to, Pieces::Knight));
        }
        else
        {
            appendMove(MoveList, board->getMove(from, to));
        }
    }
    while (captureW)
    {
        int to = popLSB(&captureW);
        int from = to - (board->sideToMove == Pieces::Color::White ? SW : NW);

        if (distToEdge[from][getDirIndex(W)] == 0)
        {
            continue;
        }

        Direction pin = board->isPinned(from);

        if (pin != NULLDIR)
        {
            if (getDirectionBetween(from, to) != pin && getDirectionBetween(to, from) != pin)
            {
                continue;
            }
        }

        if (to == board->enPassantSquare && indexToRank(kingIndex) == indexToRank(from))
        {

            clearBit(&board->pieceBB[Pieces::Pawn], from);
            clearBit(&board->colorBB[board->sideToMove], from);
            clearBit(&board->allPiecesBB, from);
            board->board[from] = Pieces::Empty;

            int capturedPawn = to - (board->sideToMove == Pieces::Color::White ? -8 : 8);
            clearBit(&board->pieceBB[Pieces::Pawn], capturedPawn);
            clearBit(&board->colorBB[board->otherSide], capturedPawn);
            clearBit(&board->allPiecesBB, capturedPawn);
            board->board[capturedPawn] = Pieces::Empty;

            // send ray from king to the pawn direction
            Direction dir = getDirectionBetween(kingIndex, from);
            Bitboard ray = sendRay(&board->allPiecesBB, dir, kingIndex);

            // Check if the ray hits an enemy sliding piece
            Bitboard sliding = (board->pieceBB[Pieces::Rook] | board->pieceBB[Pieces::Queen]) & board->colorBB[board->otherSide];

            // Undo the en passant
            setBit(&board->pieceBB[Pieces::Pawn], from);
            setBit(&board->colorBB[board->sideToMove], from);
            setBit(&board->allPiecesBB, from);
            board->board[from] = Pieces::Pawn | board->sideToMove;

            setBit(&board->pieceBB[Pieces::Pawn], capturedPawn);
            setBit(&board->colorBB[board->otherSide], capturedPawn);
            setBit(&board->allPiecesBB, capturedPawn);
            board->board[capturedPawn] = Pieces::Pawn | board->otherSide;

            if (sliding & ray)
            {
                continue;
            }
        }

        if (indexToRank(to) == 0 || indexToRank(to) == 7)
        {
            appendMove(MoveList, board->getMove(from, to, Pieces::Queen));
            appendMove(MoveList, board->getMove(from, to, Pieces::Rook));
            appendMove(MoveList, board->getMove(from, to, Pieces::Bishop));
            appendMove(MoveList, board->getMove(from, to, Pieces::Knight));
        }
        else
        {
            appendMove(MoveList, board->getMove(from, to));
        }
    }
}

void generateKnightMoves(Board *board, MoveList *moveList, bool onlyCaptures)
{
    Bitboard knights = board->pieceBB[Pieces::Knight] & board->colorBB[board->sideToMove];
    while (knights)
    {
        int knight = popLSB(&knights);
        Direction pin = board->isPinned(knight);
        if (pin != NULLDIR)
        {
            continue;
        }

        Bitboard moves = knightMoves[knight] & board->checkingBB;
        moves &= board->colorBB[board->otherSide] | board->pieceBB[0] & (onlyCaptures ? board->colorBB[board->otherSide] : -1);
        while (moves)
        {
            int to = popLSB(&moves);
            appendMove(moveList, board->getMove(knight, to));
        }
    }
}

void generateSlidingRays(Board *board, MoveList *moveList, int from, int startDir, int endDir, bool onlyCaptures)
{
    const Direction directions[8] = {N, S, E, W, NE, NW, SE, SW};
    Direction pin = board->isPinned(from);
    for (int i = startDir; i <= endDir; i++)
    {
        Direction dir = directions[i];
        if (pin != NULLDIR)
        {
            if (dir != pin && dir != invertDirection(pin))
            {
                continue;
            }
        }
        int oldF = indexToFile(from);
        int oldR = indexToRank(from);
        int to = from + dir;
        int toF = indexToFile(to);
        int toR = indexToRank(to);
        if (distToEdge[from][getDirIndex(dir)] == 0 || (Pieces::getColor(board->board[to]) == board->sideToMove && board->board[to] != Pieces::Empty))
        {
            continue;
        }

        while (to >= 0 && to < 64)
        {

            if (board->board[to] == Pieces::Empty || (Pieces::getColor(board->board[to]) != board->sideToMove && board->board[to] != Pieces::Empty))
            {
                if (!(getBitboardFromSquare(to) & board->checkingBB))
                {
                    if (board->board[to] != Pieces::Empty)
                    {
                        break;
                    }
                    if (distToEdge[to][getDirIndex(dir)] == 0)
                    {
                        break;
                    }
                    to += dir;

                    continue;
                }
                if (!onlyCaptures || board->board[to] != Pieces::Empty)
                {
                    appendMove(moveList, board->getMove(from, to));
                }
            }
            if (distToEdge[to][getDirIndex(dir)] == 0)
            {
                break;
            }
            if (board->board[to] != Pieces::Empty)
            {
                break;
            }
            oldF = indexToFile(to);
            oldR = indexToRank(to);
            to += dir;
            toF = indexToFile(to);
            toR = indexToRank(to);
        }
    }
}

void generateSlidingMoves(Board *board, MoveList *movelist, bool onlyCaptures)
{
    Bitboard rooks = board->pieceBB[Pieces::Rook] & board->colorBB[board->sideToMove];
    Bitboard bishops = board->pieceBB[Pieces::Bishop] & board->colorBB[board->sideToMove];
    Bitboard queens = board->pieceBB[Pieces::Queen] & board->colorBB[board->sideToMove];
    while (rooks)
    {
        int rook = popLSB(&rooks);
        generateSlidingRays(board, movelist, rook, 0, 3, onlyCaptures);
    }
    while (bishops)
    {
        int bishop = popLSB(&bishops);
        generateSlidingRays(board, movelist, bishop, 4, 7, onlyCaptures);
    }
    while (queens)
    {
        int queen = popLSB(&queens);
        generateSlidingRays(board, movelist, queen, 0, 7, onlyCaptures);
    }
}

void generateCastles(Board *board, MoveList *MoveList)
{
    if (board->inCheck)
    {
        return;
    }
    // Pieces in the way
    Bitboard attacked = board->attackedBB[board->otherSide];
    bool canShortCastle = board->isWhite ? board->whiteCanCastleKingSide : board->blackCanCastleKingSide;
    bool canLongCastle = board->isWhite ? board->whiteCanCastleQueenSide : board->blackCanCastleQueenSide;
    if ((shortCastle[board->sideToMove] & board->allPiecesBB) == 0 && canShortCastle)
    {
        if (!(shortCastle[board->sideToMove] & attacked))
        {
            appendMove(MoveList, board->getMove(60, board->isWhite ? 62 : 6, Pieces::Empty, true));
        }
    }
    if ((longCastle[board->sideToMove] & board->allPiecesBB) == 0 && canLongCastle)
    {
        if (!(longCastle[board->sideToMove + 1] & attacked))
        {
            appendMove(MoveList, board->getMove(4, board->isWhite ? 58 : 2, Pieces::Empty, true));
        }
    }
}

void generateKingMoves(Board *board, MoveList *MoveList, bool onlyCaptures)
{
    Bitboard king = board->pieceBB[Pieces::King] & board->colorBB[board->sideToMove];
    while (king)
    {
        int kingIndex = popLSB(&king);
        Bitboard attacked = board->attackedBB[board->otherSide];
        Bitboard moves = kingMoves[kingIndex];
        moves &= (board->colorBB[board->otherSide] | board->pieceBB[0]) & ~attacked & (onlyCaptures ? board->colorBB[board->otherSide] : -1);
        while (moves)
        {
            int to = popLSB(&moves);
            appendMove(MoveList, board->getMove(kingIndex, to));
        }
    }
}

void generateMoves(Board *board, MoveList *moveList, bool onlyCaptures)
{
    moveList->count = 0;

    generateCheckBB(board);

    // Generate moves for each piece

    generateKingMoves(board, moveList, onlyCaptures);
    generatePawnMoves(board, moveList, onlyCaptures);
    generateKnightMoves(board, moveList, onlyCaptures);
    generateSlidingMoves(board, moveList, onlyCaptures);
    if (!onlyCaptures)
    {
        generateCastles(board, moveList);
    }
}

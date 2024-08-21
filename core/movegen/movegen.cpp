#include "movegen.h"
#include "../representation/piece.h"
#include "../uci.h"


std::ostream &operator<<(std::ostream &os, const MoveList &moveList)
{
    for (int i = 0; i < moveList.count; i++)
    {
        os << moveToString(moveList.moves[i]) << " ";
    }
    return os;
}

// Generate moves for sliding pieces
template <Pieces::PieceType piece>
void generateSlidingRays(Board *board, MoveList &moveList, int from, bool onlyCaptures);

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
        if (board->isWhite)
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
            if (Pieces::getType(board->board[piece]) == Pieces::Bishop && isStraight(dir))
            {
                continue;
            }
            if (Pieces::getType(board->board[piece]) == Pieces::Rook && isDiagonal(dir))
            {
                continue;
            }
            // Check if there is a piece in between the rook and the king
            Bitboard between = bitboardRay(kingIndex, piece) & board->allPiecesBB & ~king;
            if (!between)
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

void generatePawnMoves(Board *board, MoveList &MoveList, bool onlyCaptures)
{
    // Shift the pawn BBs to get the moves
    Bitboard kingBB = board->pieceBB[Pieces::King] & board->colorBB[board->sideToMove];
    int kingIndex = getLSB(&kingBB);
    Direction up = board->isWhite ? S : N;
    Bitboard pawns = board->pieceBB[Pieces::Pawn] & board->colorBB[board->sideToMove];
    Bitboard moves;
    Bitboard singlePush;
    Bitboard doublePush;
    Bitboard captureUR;
    Bitboard captureDR;

    moves = shift(&pawns, up, 1) & board->pieceBB[0];

    if (board->isWhite)
    {
        captureUR = shift<SE>(&pawns);
        captureDR = shift<SW>(&pawns); //
    }
    else
    {
        captureUR = shift<NW>(&pawns); // Capture along the up left diagonal
        captureDR = shift<NE>(&pawns); // Capture along the down right diagonal
    }

    singlePush = moves & board->checkingBB;

    doublePush = moves & (board->isWhite ? rankMasks[5] : rankMasks[2]);
    doublePush = shift(&doublePush, up, 1) & board->pieceBB[Pieces::Empty] & board->checkingBB;
    // Add en passant to each of these
    Bitboard enPassantMask = (board->colorBB[board->otherSide]);
    Bitboard enPassantCheck = 0;
    if (board->enPassantSquare != -1)
    {
        enPassantMask |= (1ULL << board->enPassantSquare);
        // Check if the en passant will remove the king from check
        // If king is checked by pawn but not double checked
        if (board->checkers.count == 1 && Pieces::getType(board->board[board->checkers.index[0]]) == Pieces::Pawn)
        {
            int up = board->isWhite ? 8 : -8;
            if (board->checkers.index[0] - up == board->enPassantSquare)
            {
                enPassantCheck = getBitboardFromSquare(board->enPassantSquare);
            }
        }
    }
    captureUR &= enPassantMask;
    captureDR &= enPassantMask;

    // In check
    captureUR &= board->checkingBB | enPassantCheck;
    captureDR &= board->checkingBB | enPassantCheck;

    // Decode BBs into moves
    if (!onlyCaptures)
    {
        while (singlePush)
        {

            int to = popLSB(&singlePush);
            int from = to - up;

            Direction pin = board->isPinned(from);

            if (pin != N && pin != S && pin != NULLDIR)
            {
                clearBit(&doublePush, to + up); // Remove the double push if the pawn is pinned to save time later
                continue;
            }

            if (indexToRank(to) == 0 || indexToRank(to) == 7)
            {
                MoveList += board->getMove(from, to, Pieces::Queen);
                MoveList += board->getMove(from, to, Pieces::Rook);
                MoveList += board->getMove(from, to, Pieces::Bishop);
                MoveList += board->getMove(from, to, Pieces::Knight);
            }
            else
            {
                MoveList += board->getMove(from, to);
            }
        }
        // Double pawn push
        while (doublePush)
        {
            int to = popLSB(&doublePush);
            int from = to - (up << 1); // from = to - up - up

            if (getBitboardFromSquare(from + up) & ~board->checkingBB)
            {
                // The pawn was not checked for pins yet by the single pawn push
                // This is because the single pawn  pushes are filtered by the checking BB, so if the king is in check
                // and a pawn can't block it with a single push, then then the pawn will skip the single push
                // and go straight to the double push, so we need to check for pins here
                Direction pin = board->isPinned(from);
                if (pin != N && pin != S && pin != NULLDIR)
                {
                    continue;
                }
            }

            MoveList += board->getMove(from, to);
            
        }
    }

    Direction moveDirUR = (board->isWhite ? SE : NW);
    while (captureUR)
    {
        int to = popLSB(&captureUR);
        int from = to - moveDirUR;

        if (distToEdge[from][getDirIndex(moveDirUR)] == 0)
        {
            continue;
        }
        Direction pin = board->isPinned(from);

        if (pin != SE && pin != NW && pin != NULLDIR)
        {
            continue;
        }
        // Handle en passant by removing the pieces from the bitboard and checking if the king is under attack

        if (to == board->enPassantSquare && indexToRank(kingIndex) == indexToRank(from))
        {

            clearBit(&board->pieceBB[Pieces::Pawn], from);
            clearBit(&board->colorBB[board->sideToMove], from);
            clearBit(&board->allPiecesBB, from);
            board->board[from] = Pieces::Empty;

            int capturedPawn = to - (board->isWhite ? -8 : 8);
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
            MoveList += board->getMove(from, to, Pieces::Queen | board->sideToMove);
            MoveList += board->getMove(from, to, Pieces::Rook | board->sideToMove);
            MoveList += board->getMove(from, to, Pieces::Bishop | board->sideToMove);
            MoveList += board->getMove(from, to, Pieces::Knight | board->sideToMove);
        }
        else
        {
            MoveList += board->getMove(from, to);
        }
    }
    Direction moveDirDR = (board->isWhite ? SW : NE);
    while (captureDR)
    {

        int to = popLSB(&captureDR);
        int from = to - moveDirDR;

        if (distToEdge[from][getDirIndex(moveDirDR)] == 0)
        {
            continue;
        }

        Direction pin = board->isPinned(from);

        if (pin != SW && pin != NE && pin != NULLDIR)
        {
            continue;
        }

        if (to == board->enPassantSquare && indexToRank(kingIndex) == indexToRank(from))
        {

            clearBit(&board->pieceBB[Pieces::Pawn], from);
            clearBit(&board->colorBB[board->sideToMove], from);
            clearBit(&board->allPiecesBB, from);
            board->board[from] = Pieces::Empty;

            int capturedPawn = to - (board->isWhite ? -8 : 8);
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
            MoveList += board->getMove(from, to, Pieces::Queen | board->sideToMove);
            MoveList += board->getMove(from, to, Pieces::Rook | board->sideToMove);
            MoveList += board->getMove(from, to, Pieces::Bishop | board->sideToMove);
            MoveList += board->getMove(from, to, Pieces::Knight | board->sideToMove);
        }
        else
        {
            MoveList += board->getMove(from, to);
        }
    }
}

void generateKnightMoves(Board *board, MoveList &moveList, bool onlyCaptures)
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
            moveList += board->getMove(knight, to);
        }
    }
}

template <>
void generateSlidingRays<Pieces::Bishop>(Board *board, MoveList &moveList, int from, bool onlyCaptures)
{
    Direction pin = board->isPinned(from);

    Bitboard blockers = bishopMasks[from] & board->allPiecesBB;
    Bitboard moves = 0;

    Key key = getKey(blockers, &bishopMagics[from]);
    Bitboard magicMoves = bishopMagics[from].table[key];

    moves = magicMoves & board->checkingBB;

    if (isStraight(pin))
    {
        // If the bishop is pinned in a straight direction, it can't move
        moves = 0ULL;
        return;
    }
    else if (pin != NULLDIR)
        moves &= dirToBB[getDirIndex(pin)][from] | dirToBB[getDirIndex(~pin)][from];

    moves &= board->colorBB[board->otherSide] | (onlyCaptures ? 0ULL : board->pieceBB[Pieces::Empty]);

    while (moves)
    {
        int to = popLSB(&moves);
        moveList += board->getMove(from, to);
    }
}

template <>
void generateSlidingRays<Pieces::Rook>(Board *board, MoveList &moveList, int from, bool onlyCaptures)
{
    Direction pin = board->isPinned(from);

    Bitboard blockers = rookMasks[from] & board->allPiecesBB;
    Bitboard moves = 0;

    Key key = getKey(blockers, &rookMagics[from]);
    Bitboard magicMoves = rookMagics[from].table[key];

    moves = magicMoves & board->checkingBB;

    if (isDiagonal(pin))
    {
        // If the Rook is pinned in a diagonal direction, it can't move
        moves = 0ULL;
        return;
    }
    else if (pin)
        moves &= dirToBB[getDirIndex(pin)][from] | dirToBB[getDirIndex(~pin)][from];

    moves &= board->colorBB[board->otherSide] | (onlyCaptures ? 0ULL : board->pieceBB[Pieces::Empty]);

    while (moves)
    {
        int to = popLSB(&moves);
        moveList += board->getMove(from, to);
    }
}

template <>
void generateSlidingRays<Pieces::Queen>(Board *board, MoveList &moveList, int from, bool onlyCaptures)
{
    Direction pin = board->isPinned(from);

    Bitboard rookBlockers = rookMasks[from] & board->allPiecesBB;
    Bitboard bishopBlockers = bishopMasks[from] & board->allPiecesBB;
    Bitboard moves = 0;

    Key rookKey = getKey(rookBlockers, &rookMagics[from]);
    Bitboard magicMoves = rookMagics[from].table[rookKey];

    Key bishopKey = getKey(bishopBlockers, &bishopMagics[from]);
    magicMoves |= bishopMagics[from].table[bishopKey];

    moves = magicMoves & board->checkingBB;

    if (pin) // If Queen is pinned
    {
        moves &= dirToBB[getDirIndex(pin)][from] | dirToBB[getDirIndex(~pin)][from];
    }
    moves &= board->colorBB[board->otherSide] | (onlyCaptures ? 0ULL : board->pieceBB[Pieces::Empty]);

    while (moves)
    {
        int to = popLSB(&moves);
        moveList += board->getMove(from, to);
    }
}

void generateSlidingMoves(Board *board, MoveList &movelist, bool onlyCaptures)
{
    Bitboard rooks = board->pieceBB[Pieces::Rook] & board->colorBB[board->sideToMove];
    Bitboard bishops = board->pieceBB[Pieces::Bishop] & board->colorBB[board->sideToMove];
    Bitboard queens = board->pieceBB[Pieces::Queen] & board->colorBB[board->sideToMove];
    while (bishops)
    {
        int bishop = popLSB(&bishops);
        generateSlidingRays<Pieces::Bishop>(board, movelist, bishop, onlyCaptures);
    }
    while (rooks)
    {
        int rook = popLSB(&rooks);
        generateSlidingRays<Pieces::Rook>(board, movelist, rook, onlyCaptures);
    }
    while (queens)
    {
        int queen = popLSB(&queens);
        generateSlidingRays<Pieces::Queen>(board, movelist, queen, onlyCaptures);
    }
}

void generateCastles(Board *board, MoveList &MoveList)
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
            MoveList += board->getMove(board->isWhite ? 60 : 4, board->isWhite ? 62 : 6, Pieces::Empty, true);
        }
    }
    if ((longCastle[board->sideToMove] & board->allPiecesBB) == 0 && canLongCastle)
    {
        if (!(longCastle[board->sideToMove + 1] & attacked))
        {
            MoveList += board->getMove(board->isWhite ? 60 : 4, board->isWhite ? 58 : 2, Pieces::Empty, true);
        }
    }
}

void generateKingMoves(Board *board, MoveList &MoveList, bool onlyCaptures)
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
            MoveList += board->getMove(kingIndex, to);
        }
    }
}

void generateMoves(Board *board, MoveList &moveList, bool onlyCaptures)
{
    moveList.count = 0;

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

#include <cassert>

#include "board.h"
#include "bitboard.h"
#include "../uci.h"
#include "../movegen/movegen.h"



// Resets bitboards based on the board array

int invertX(int i)
{
    int x = 7 - i % 8;
    int y = i / 8;
    return x + y << 3;
}

int flipSide(int i)
{
    int x = indexToFile(i);
    int y = indexToRank(i);
    return x + y << 3; // Reverting by not using 7 -
}

// appends a bitboard of pieces to a list of indexes
void appendBB(indexList *list, Bitboard BB)
{
    while (BB)
    {
        int index = popLSB(&BB);
        list->index[list->count] = index;
        list->count++;
    }
}

void Board::setupBitboards()
{
    checkingBB = 0xFFFFFFFFFFFFFFFFULL;
    for (int i = 0; i < 7; i++)
    {
        pieceBB[i] = 0;
    }
    colorBB[0] = 0;
    colorBB[8] = 0;

    int current = 0;
    for (Piece x : board)
    {
        // Get piece info
        Pieces::Color side = Pieces::getColor(x);
        Pieces::PieceType type = Pieces::getType(x);

        // Set bitboards
        setBit(&pieceBB[type], current);
        setBit(&colorBB[side], current);
        current++;
    }
    allPiecesBB = colorBB[0] | colorBB[8];
}

// Adds a piece to the board (Does not update allpieces bitboard)
void Board::setPiece(Piece piece, int square)
{
    Pieces::Color side = Pieces::getColor(piece);
    Pieces::PieceType type = Pieces::getType(piece);

    // Set bitboards
    clearBit(&pieceBB[Pieces::Empty], square);
    setBit(&pieceBB[type], square);
    setBit(&colorBB[side], square);

    // Zobrist hash key
    zobristKey ^= Zobrist::piece[Pieces::Empty][square];
    zobristKey ^= Zobrist::piece[piece][square];

    board[square] = piece;
}

// Removes a piece from the board (Does not update allpieces bitboard)
void Board::removePiece(int square)
{
    Piece piece = board[square];
    Pieces::Color side = Pieces::getColor(piece);
    Pieces::PieceType type = Pieces::getType(piece);

    // Set bitboards
    clearBit(&pieceBB[type], square);
    clearBit(&colorBB[side], square);
    setBit(&pieceBB[Pieces::Empty], square);

    // Zobrist hash key
    zobristKey ^= Zobrist::piece[piece][square];
    zobristKey ^= Zobrist::piece[Pieces::Empty][square];

    board[square] = Pieces::Empty;
}

void Board::loadFEN(const string &fen, bool isWhite, bool whiteCanCastleKingSide,
                    bool whiteCanCastleQueenSide, bool blackCanCastleKingSide,
                    bool blackCanCastleQueenSide, int enPassantSquare)
{
    int current = 0;
    this->zobristKey = 0;


    for (auto x : fen)
    {
        if (x == '/')
        {
            // Skip if newline
            continue;
        }
        if (!isalpha(x))
        {
            for (int i = 0; i < x - '0'; i++)
            {
                board[current] = Pieces::Empty;
                current++;
            }
        }
        else
        {
            Piece type = Pieces::charToPiece(x);
            setPiece(type, current);
            current++;
        }
    }

    this->ply = 0;
    this->isWhite = isWhite;
    this->sideToMove = isWhite ? Pieces::White : Pieces::Black;
    this->otherSide = isWhite ? Pieces::Black : Pieces::White;
    this->blackCanCastleKingSide = blackCanCastleKingSide;
    this->blackCanCastleQueenSide = blackCanCastleQueenSide;
    this->whiteCanCastleKingSide = whiteCanCastleKingSide;
    this->whiteCanCastleQueenSide = whiteCanCastleQueenSide;
    this->removeCastlingRightsBK = -1;
    this->removeCastlingRightsBQ = -1;
    this->removeCastlingRightsWK = -1;
    this->removeCastlingRightsWQ = -1;

    enPassantSquare = enPassantSquare;
    setupBitboards();
    attackedBB[sideToMove] = getAttackedBB(sideToMove);
    attackedBB[otherSide] = getAttackedBB(otherSide);
    inCheck = attackedBB[otherSide] & pieceBB[Pieces::King] & colorBB[sideToMove];
}

void Board::printFEN()
{
    cout << "FEN: ";

    int open = 0;
    for (int rank = 7; rank >= 0; rank--)
    {

        if (open)
        {
            std::cout << open;
            open = 0;
        }

        for (int file = 0; file < 8; file++)
        {
            Piece piece = board[rankFileToIndex(rank, file)];
            if (piece)
            {
                if (open)
                {
                    std::cout << open;
                    open = 0;
                }
                std::cout << pieceToChar(piece);
            }
            else
            {
                open += 1;
            }
        }
        std::cout << "/";
    }

    cout << (isWhite ? " w " : " b ");
    cout << (whiteCanCastleKingSide ? "K" : "");
    cout << (whiteCanCastleQueenSide ? "Q" : "");
    cout << (blackCanCastleKingSide ? "k" : "");
    cout << (blackCanCastleQueenSide ? "q" : "");
    cout << endl;
}

// Set's a move on the board and updates the bitboards
void Board::setMove(Move move)
{
    int from = getFrom(move);
    int to = getTo(move);

    Piece movePiece = board[from];
    Piece capturedPiece = getCapturedPiece(move);

    assert(Pieces::getType(capturedPiece) != Pieces::King);

    removePiece(from);

    removePiece(to); // Remove the captured piece

    setPiece(movePiece, to);
}

void Board::revertSetMove(Move move)
{
    int from = getFrom(move);
    int to = getTo(move);
    Piece movePiece = board[to];
    Piece capturedPiece = getCapturedPiece(move);

    removePiece(to);

    setPiece(capturedPiece, to);

    setPiece(movePiece, from);
}

Bitboard Board::getPieceBB(Piece piece)
{
    return pieceBB[Pieces::getType(piece)] & colorBB[Pieces::getColor(piece)];
}

Bitboard Board::getAttackedBB(Pieces::Color side)
{
    Bitboard attackedBB = 0;

    Bitboard pawnBB = pieceBB[Pieces::Pawn] & colorBB[side];
    if (side == Pieces::White)
    {
        Bitboard pawnsBB_ = pawnBB & ~fileMasks[7];
        attackedBB |= shift(&pawnsBB_, SE, 1);
        pawnsBB_ = pawnBB & ~fileMasks[0];
        attackedBB |= shift(&pawnsBB_, SW, 1);
    }
    else
    {
        Bitboard pawnsBB_ = pawnBB & ~fileMasks[7];
        attackedBB |= shift(&pawnsBB_, NE, 1);
        pawnsBB_ = pawnBB & ~fileMasks[0];
        attackedBB |= shift(&pawnsBB_, NW, 1);
    }

    // Knights
    Bitboard knightBB = pieceBB[Pieces::Knight] & colorBB[side];

    while (knightBB)
    {
        attackedBB |= getAttackBB<Pieces::Knight>(popLSB(&knightBB));
    }

    // Bishops and Queens

    Bitboard noKings = allPiecesBB & ~(pieceBB[Pieces::King] &
                                       colorBB[Pieces::invertColor(side)]);

    Bitboard bishopBB =
        (pieceBB[Pieces::Bishop] | pieceBB[Pieces::Queen]) & colorBB[side];

    while (bishopBB)
    {
        attackedBB |= getAttackBB<Pieces::Bishop>(popLSB(&bishopBB), &noKings);
    }

    // Rooks and Queens
    Bitboard rookBB =
        (pieceBB[Pieces::Rook] | pieceBB[Pieces::Queen]) & colorBB[side];

    while (rookBB)
    {
        attackedBB |= getAttackBB<Pieces::Rook>(popLSB(&rookBB), &noKings);
    }

    // Kings
    Bitboard kingBB = pieceBB[Pieces::King] & colorBB[side];
    int kingSquare = getLSB(&kingBB);
    attackedBB |= getAttackBB<Pieces::King>(kingSquare);

    return attackedBB;
}

indexList Board::piecesAttackingSquare(int square)
{
    // The idea is toz check the squares that the king can be attacked from
    // If an enemy piece is on one of those squares, the king is in check

    // Get king square
    // Note that king square means whatever square is specified, not necessarily
    // the king
    indexList attackers;
    attackers.count = 0;
    Bitboard kingBB = getBitboardFromSquare(square);
    int kingSquare = getLSB(&kingBB);

    // Get all enemy pieces
    Bitboard enemyPieces = colorBB[otherSide];
    Bitboard checkBB;
    // Check for pawns
    Bitboard pawnBB = pieceBB[Pieces::Pawn] & enemyPieces;
    if (sideToMove == Pieces::White)
    {
        pawnBB = shift(&pawnBB, NE, 1);
    }
    else
    {
        pawnBB = shift(&pawnBB, SW, 1);
    }
    if (kingBB & pawnBB)
    {
        checkBB = kingBB & pawnBB;
        appendBB(&attackers, checkBB);
    }
    pawnBB = pieceBB[Pieces::Pawn] & enemyPieces;
    if (sideToMove == Pieces::White)
    {
        pawnBB = shift(&pawnBB, NW, 1);
    }
    else
    {
        pawnBB = shift(&pawnBB, SE, 1);
    }
    if (kingBB & pawnBB)
    {
        checkBB = kingBB & pawnBB;
        appendBB(&attackers, checkBB);
    }

    // Check for knights
    Bitboard knightBB = pieceBB[Pieces::Knight] & enemyPieces;
    Direction knightDirs[] = {NNE, NNW, SSE, SSW, NEE, NWW, SEE, SWW};

    for (int i = 0; i < 8; i++)
    {
        Bitboard bb = shift(&knightBB, knightDirs[i], 1);
        if (kingBB & bb)
        {
            checkBB = kingBB & bb;
            appendBB(&attackers, checkBB);
        }
    }

    // Check for bishops and queens
    Bitboard bishopBB =
        (pieceBB[Pieces::Bishop] | pieceBB[Pieces::Queen]) & enemyPieces;
    Direction bishopDirs[] = {NE, NW, SE, SW};

    while (bishopBB)
    {
        int bishopSquare = popLSB(&bishopBB);
        for (int i = 0; i < 4;
             i++)
        { // Might need to change bishopBB back to allpieceBB
            Bitboard checkBB = sendRay(&allPiecesBB, bishopDirs[i], bishopSquare);
            if (checkBB & kingBB)
            {
                checkBB = kingBB & checkBB;
                appendBB(&attackers, checkBB);
            }
        }
    }

    // Check for rooks and queens
    Direction attackingDirSliding[] = {N, E, S, W};
    Bitboard rookBB =
        (pieceBB[Pieces::Rook] | pieceBB[Pieces::Queen]) & enemyPieces;
    while (rookBB)
    {
        int rookSquare = popLSB(&rookBB);
        for (int i = 0; i < 4; i++)
        {
            Bitboard checkBB = sendRay(&rookBB, attackingDirSliding[i], rookSquare);
            if (checkBB & kingBB)
            {
                checkBB = kingBB & checkBB;
                appendBB(&attackers, checkBB);
            }
        }
    }

    return attackers;
}

indexList Board::getCheckers()
{
    Bitboard kingBB = colorBB[sideToMove] & pieceBB[Pieces::King];
    return piecesAttackingSquare(getLSB(&kingBB));
}

// Plays a move on the board
void Board::makeMove(Move move)
{

    int from = getFrom(move);
    int to = getTo(move);
    Piece movePiece = board[from];

    if (isPromotion(move)) // If the move is a promotion
    {
        // this code turns a promoting pawn into its promotion,
        // but doesn't move it to the to square yet, this is done later with the setMove function

        Piece promotion = getPromotion(move) | sideToMove;

        removePiece(from);
        setPiece(promotion, from);
    }

    if (Pieces::isKing(board[getFrom(move)]) && (isWhite ? (whiteCanCastleKingSide | whiteCanCastleQueenSide)
                                                         : (blackCanCastleKingSide | blackCanCastleQueenSide)))
    {
        if (sideToMove == Pieces::White && move)
        {
            whiteCanCastleKingSide = false;
            whiteCanCastleQueenSide = false;
            if (removeCastlingRightsWK == -1)
            {
                removeCastlingRightsWK = ply;
            }
            if (removeCastlingRightsWQ == -1)
            {
                removeCastlingRightsWQ = ply;
            }
        }
        else
        {
            blackCanCastleKingSide = false;
            blackCanCastleQueenSide = false;
            if (removeCastlingRightsBK == -1)
            {
                removeCastlingRightsBK = ply;
            }
            if (removeCastlingRightsBQ == -1)
            {
                removeCastlingRightsBQ = ply;
            }
        }
    }
    // Save the move to the history

    pastMoves[ply] = move;

    // Update the board

    // Remove castling rights if a rook moves and update the castling rights history ply
    if (from == 0 &&
        board[from] == (Pieces::Black | Pieces::Rook) &&
        blackCanCastleQueenSide)
    {
        blackCanCastleQueenSide = false;
        removeCastlingRightsBQ = ply;
    }
    if (from == 7 &&
        board[from] == (Pieces::Black | Pieces::Rook) &&
        blackCanCastleKingSide)
    {
        blackCanCastleKingSide = false;
        removeCastlingRightsBK = ply;
    }
    if (from == 56 &&
        board[from] == (Pieces::White | Pieces::Rook) &&
        whiteCanCastleQueenSide)
    {
        whiteCanCastleQueenSide = false;
        removeCastlingRightsWQ = ply;
    }
    if (from == 63 &&
        board[from] == (Pieces::White | Pieces::Rook) &&
        whiteCanCastleKingSide)
    {
        whiteCanCastleKingSide = false;
        removeCastlingRightsWK = ply;
    }

    // Remove castling rights if a rook is captured
    if (to == 0 &&
        board[to] == (Pieces::Black | Pieces::Rook) &&
        blackCanCastleQueenSide)
    {
        blackCanCastleQueenSide = false;
        removeCastlingRightsBQ = ply;
    }
    if (to == 7 &&
        board[to] == (Pieces::Black | Pieces::Rook) &&
        blackCanCastleKingSide)
    {
        blackCanCastleKingSide = false;
        removeCastlingRightsBK = ply;
    }
    if (to == 56 &&
        board[to] == (Pieces::White | Pieces::Rook) &&
        whiteCanCastleQueenSide)
    {
        whiteCanCastleQueenSide = false;
        removeCastlingRightsWQ = ply;
    }
    if (to == 63 &&
        board[to] == (Pieces::White | Pieces::Rook) &&
        whiteCanCastleKingSide)
    {
        whiteCanCastleKingSide = false;
        removeCastlingRightsWK = ply;
    }

    // Remove castling rights if castle
    if (isCastle(move))
    {
        if (sideToMove == Pieces::White)
        {
            whiteCanCastleKingSide = false;
            whiteCanCastleQueenSide = false;
            if (removeCastlingRightsWK == -1)
            {
                removeCastlingRightsWK = ply;
            }
            if (removeCastlingRightsWQ == -1)
            {
                removeCastlingRightsWQ = ply;
            }
        }
        else
        {

            blackCanCastleKingSide = false;
            blackCanCastleQueenSide = false;
            if (removeCastlingRightsBK == -1)
            {
                removeCastlingRightsBK = ply;
            }
            if (removeCastlingRightsBQ == -1)
            {
                removeCastlingRightsBQ = ply;
            }
        }

        // Set the pieces to the correct squares in order to castle
        if (to == 62)
        {
            setMove(getMove(60, 62));
            setMove(getMove(63, 61));
        }
        if (to == 58)
        {
            setMove(getMove(60, 58));
            setMove(getMove(56, 59));
        }
        if (to == 6)
        {
            setMove(getMove(4, 6));
            setMove(getMove(7, 5));
        }
        if (to == 2)
        {
            setMove(getMove(4, 2));
            setMove(getMove(0, 3));
        }
    }
    else if (isEnPassant(move))
    {
        setMove(move);

        // Remove enemy pawn
        int enemyPawn = getTo(move) + (isWhite ? N : S);

        removePiece(enemyPawn);
    }
    else
    {
        setMove(move);
        enPassantHash = 0;
    }

    enPassantHistory[ply] = enPassantSquare;
    enPassantSquare = -1;

    // Update en passant square for next move
    if (Pieces::isPawn(movePiece))
    {
        if (abs(from - to) == 16)
        {
            if (sideToMove == Pieces::White)
            {
                enPassantSquare = from + S;
            }
            else
            {
                enPassantSquare = from + N;
            }
        }
    }

    // Update ply
    ply++;
    // Update side to move
    sideToMove = ~sideToMove;
    otherSide = static_cast<Pieces::Color>(sideToMove * -1 + 8);
    isWhite = Pieces::isWhite(sideToMove);

    // Update allpiece bitboard
    allPiecesBB = colorBB[Pieces::White] | colorBB[Pieces::Black];

    // Update checking bitboard
    attackedBB[sideToMove] = getAttackedBB(sideToMove);
    attackedBB[otherSide] = getAttackedBB(otherSide);

    inCheck = attackedBB[otherSide] & pieceBB[Pieces::King] & colorBB[sideToMove];

    zobristKey ^= Zobrist::side[isWhite];
    zobristKey ^= Zobrist::side[!isWhite];
}

void Board::undoMove()
{
    // Update ply
    ply--;
    // Update side to move
    sideToMove = ~sideToMove;
    otherSide = static_cast<Pieces::Color>(sideToMove * -1 + 8);
    isWhite = Pieces::isWhite(sideToMove);

    // Update en passant square
    enPassantSquare = enPassantHistory[ply];
    if (isPromotion(pastMoves[ply]))
    {
        // Make the promoted piece a pawn
        int to = getTo(pastMoves[ply]);

        removePiece(to);
        setPiece(Pieces::Pawn | sideToMove, to);
    }

    // Update castling rights
    if (isCastle(pastMoves[ply]))
    {
        if (getTo(pastMoves[ply]) == 62)
        {
            revertSetMove(getMove(60, 62, Pieces::Empty, true));
            revertSetMove(getMove(63, 61, Pieces::Empty, true));
        }
        if (getTo(pastMoves[ply]) == 58)
        {
            revertSetMove(getMove(60, 58, Pieces::Empty, true));
            revertSetMove(getMove(56, 59, Pieces::Empty, true));
        }
        if (getTo(pastMoves[ply]) == 6)
        {
            revertSetMove(getMove(4, 6, Pieces::Empty, true));
            revertSetMove(getMove(7, 5, Pieces::Empty, true));
        }
        if (getTo(pastMoves[ply]) == 2)
        {
            revertSetMove(getMove(4, 2, Pieces::Empty, true));
            revertSetMove(getMove(0, 3, Pieces::Empty, true));
        }
    }
    else
    {
        revertSetMove(pastMoves[ply]); // Reverts the move
        enPassantHash = 0;
    }

    if (isEnPassant(pastMoves[ply]))
    {
        int enemyPawn = getTo(pastMoves[ply]) + (isWhite ? 8 : -8);

        setPiece(Pieces::Pawn | otherSide, enemyPawn);
    }

    if (ply == removeCastlingRightsWK)
    {
        whiteCanCastleKingSide = true;
        removeCastlingRightsWK = -1;
    }
    if (ply == removeCastlingRightsBK)
    {
        blackCanCastleKingSide = true;
        removeCastlingRightsBK = -1;
    }
    if (ply == removeCastlingRightsWQ)
    {
        whiteCanCastleQueenSide = true;
        removeCastlingRightsWQ = -1;
    }
    if (ply == removeCastlingRightsBQ)
    {
        blackCanCastleQueenSide = true;
        removeCastlingRightsBQ = -1;
    }
    // Update the board

    allPiecesBB = colorBB[0] | colorBB[8]; // update all pieces bitboard

    // Update checking bitboard

    attackedBB[sideToMove] = getAttackedBB(sideToMove);
    attackedBB[otherSide] = getAttackedBB(otherSide);

    inCheck = attackedBB[otherSide] & pieceBB[Pieces::King] & colorBB[sideToMove];
    castleKey = blackCanCastleKingSide << 3 | blackCanCastleQueenSide << 2 |
                whiteCanCastleKingSide << 1 | whiteCanCastleKingSide << 0;

    zobristKey ^= Zobrist::side[isWhite];
    zobristKey ^= Zobrist::side[!isWhite];
}

bool Board::isCheck(Move move)
{
    makeMove(move);
    bool check = inCheck;
    undoMove();
    return check;
}

Direction Board::isPinned(int square)
{
    // Get king square
    Bitboard kingBB = colorBB[sideToMove] & pieceBB[Pieces::King];
    if (!kingBB)
    {
        printFEN();
    }
    int kingSquare = getLSB(&kingBB);

    // Get all enemy pieces
    Bitboard enemyPieces = colorBB[otherSide];

    // Is the direction between the king and the square a direction which sliding pieces can attack?
    Direction dir = getDirectionBetween(kingSquare, square);
    if (dir != NULLDIR && dir > -10 && dir < 10) // the direction is diagonal or straight
    {
        Bitboard checkBB = bitboardRay(kingSquare, square) & allPiecesBB & ~(kingBB);
        // If there is not a piece in between the king and the square
        if (!checkBB)
        {
            // If direction is diagonal
            if (isDiagonal(dir))
            {
                Bitboard attackers = sendRay(&allPiecesBB, dir, square);
                if (attackers & ((pieceBB[Pieces::Bishop] | pieceBB[Pieces::Queen]) &
                                 enemyPieces))
                {
                    return dir;
                }
            }
            if (isStraight(dir))
            {
                Bitboard attackers = sendRay(&allPiecesBB, dir, square);
                if (attackers &
                    ((pieceBB[Pieces::Rook] | pieceBB[Pieces::Queen]) & enemyPieces))
                {
                    return dir;
                }
            }
        }
    }
    return Direction::NULLDIR;
}

bool Board::isAttacked(int square, Pieces::Color side)
{
    return getBitboardFromSquare(square) & attackedBB[side];
}

Move Board::getMove(int from, int to, Piece promotion, bool isCastle)
{
    // Move is formatted as follows:
    // 0000 0000 000000 000000
    // ECCP PROM   TO    FROM
    // ECCP: En Passant, Castle, Capture, Promotion
    Move move = 0;
    // Set from
    move |= from;
    // Set to
    move |= to << 6;
    // Set promotion
    move |= promotion << 12;
    // Set isPromote
    move |= (promotion > 0) << 17;
    // Set isCapture
    move |= (isCastle ? 0 : (board[to] != 0)) << 18;
    // Set isCastle
    move |= isCastle << 19;
    // Set captured piece
    move |= (isCastle ? 0 : board[to] & 0xF) << 20;
    return move;
}

bool Board::isEnPassant(Move move)
{
    return getTo(move) == enPassantSquare &&
           Pieces::getType(board[getFrom(move)]) == Pieces::Pawn;
}

Board::Board()
{
    ply = 0;
    sideToMove = Pieces::White;
    otherSide = Pieces::Black;
    blackCanCastleKingSide = true;
    blackCanCastleQueenSide = true;
    whiteCanCastleKingSide = true;
    whiteCanCastleQueenSide = true;
    enPassantSquare = -1;
    isWhite = Pieces::isWhite(sideToMove);

    zobristKey ^= Zobrist::castle[15] ^ Zobrist::side[true];
}

void Board::clearBoard()
{
    for (int i = 0; i < 64; i++)
    {
        board[i] = 0;
    }
}

void Board::printBoard()
{
    for (int rank = 0; rank < 8; rank++)
    {
        cout << "\n+---+---+---+---+---+---+---+---+" << endl;
        cout << "| ";
        for (int file = 0; file < 8; file++)
        {
            cout << Pieces::pieceToChar(board[rankFileToIndex(rank, file)]) << " | ";
        }

        cout << 7 - rank + 1;
    }
    cout << "\n"
         << "+---+---+---+---+---+---+---+---+\n"
         << "  a   b   c   d   e   f   g   h  " << endl;
}
namespace Zobrist
{
    unsigned long long piece[15][64];
    unsigned long long side[2];
    unsigned long long castle[16];
    unsigned long long enPassant[8];
    void setup()
    {
        RNG rng;

        for (int p = 0; p < 15; p++)
        {
            for (int i = 0; i < 64; i++)
            {
                piece[p][i] = rng.rand64();
            }
        }
        side[0] = rng.rand64();
        side[1] = rng.rand64();
        for (int c = 0; c < 16; c++)
        {
            castle[c] = rng.rand64();
        }
        for (int e = 0; e < 8; e++)
        {
            enPassant[e] = rng.rand64();
        }
    }
}
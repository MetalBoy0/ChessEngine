#ifndef UCI_H
#define UCI_H

#include <iostream>
#include <sstream>

#include "representation/board.h"
#include "search/search.h"

#define MAX_INT -1u
#define MAX_DEPTH 64

using namespace std;

#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a > b) ? a : b)



extern Move stringToMove(string moveString, Board board);
extern string moveToString(Move move);
extern char pieceToChar(Piece piece);
extern void parseUCI(istringstream &parser);      // Handles the "UCI" command
extern void parseIsReady(istringstream &parser);  // Handles the "isready" command
extern void parseNewGame(istringstream &parser);  // Handles the "ucinewgame" command
extern void parsePosition(istringstream &parser); // Handles the "position" command
extern void parseGo(istringstream &parser);       // Handles the "go" command
extern void parseEval(istringstream &parser);      // Handles the "eval" command
extern void parseDebug(istringstream &parser);    // Handles the "debug" command
extern void parseDisplay(istringstream &parser);  // Handles the "d" command
extern void parseMakeMove(istringstream &parser); // Handles the "makemove" command
extern void parseUndoMove(istringstream &parser); // Handles the "undomove" command
extern void parseClearTT(istringstream &parser);     // Handles the "clearTT" command
extern void setup();                              // set up the uci and other relevant variables


#endif
#ifndef UCI_H
#define UCI_H

#include <iostream>
#include <sstream>

#include "core\representation\board.h"
#include "core\Search\search.h"

using namespace std;

extern Move stringToMove(string moveString, Board board);
extern string moveToString(Move move);
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
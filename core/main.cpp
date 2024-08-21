/*
Pioneer V0.3
Will Garrison
TODO: Ensure it is uci compliant
*/

#include "uci.h" // uci.cpp will handle the communication between the user and the engine

using namespace std;

int main()
{

    setup(); // Setup the board

    std::cout << "Welcome the Pioneer V0.3, a chess engine by Will Garrison\n";

    std::string input;
    while (true)
    {
        // Get the input from the user
        getline(std::cin, input);

        // string parser
        istringstream parser(input);
        parser >> input;

        // Begin parsing the input
        if (input == "quit")
        {
            break; // quit the program
        }

        else if (input == "uci")
        {
            parseUCI(parser);
        }

        else if (input == "isready")
        {
            parseIsReady(parser);
        }

        else if (input == "ucinewgame")
        {
            parseNewGame(parser);
        }

        else if (input == "position")
        {
            parsePosition(parser);
        }

        else if (input == "go")
        {
            parseGo(parser);
        }

        else if (input == "debug")
        {
            parseDebug(parser);
        }
        else if (input == "d")
        {
            parseDisplay(parser);
        }
        else if (input == "makemove")
        {
            parseMakeMove(parser);
        }
        else if (input == "undomove")
        {
            parseUndoMove(parser);
        }
        else if (input == "eval")
        {
            parseEval(parser);
        }
        else if (input == "clear")
        {
            parseClearTT(parser);
        }
        else
        {
            cout << "Unknown command: " << input << endl;
        }

        // Clear the input
        input.clear();
    }
}

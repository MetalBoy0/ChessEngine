// My third try at my cpp chess ai, Pioneer.
// Will Garrison
// TODO: Ensure it is uci compliant
// TODO: Get a working move generator
// TODO: Move generation must support 10 mN/S (Perft up to depth of 5 in < 1 second)
// TODO: Include Bitboards and eventually magic bitboards

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
            //cout << "\n-\n";
        }

        else if (input == "isready")
        {
            parseIsReady(parser);
            //cout << "\n-\n";
        }

        else if (input == "ucinewgame")
        {
            parseNewGame(parser);
            //cout << "\n-\n";
        }

        else if (input == "position")
        {
            parsePosition(parser);
            //cout << "\n-\n";
        }

        else if (input == "go")
        {
            parseGo(parser);
            //cout << "\n-\n";
        }

        else if (input == "debug")
        {
            parseDebug(parser);
            //cout << "\n-\n";
        }
        else if (input == "d")
        {
            parseDisplay(parser);
            //cout << "\n-\n";
        }
        else if (input == "makemove")
        {
            parseMakeMove(parser);
            //cout << "\n-\n";
        }
        else if (input == "undomove")
        {
            parseUndoMove(parser);
            //cout << "\n-\n";
        }
        else if (input == "eval")
        {
            parseEval(parser);
            //cout << "\n-\n";
        }
        else if (input == "clear") {
            parseClearTT(parser);
        }
        else
        {
            cout << "Unknown command: " << input;
        }
        cout << "\n";
    }
}

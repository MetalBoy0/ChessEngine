#include <list>
#include <cmath>
#include <random>
#include <iostream>
#include <fstream>

#include "magicBB.h"
#include "board.h"


typedef unsigned long long Key;

constexpr Bitboard edges = rankMasks[0] | rankMasks[7] | fileMasks[0] | fileMasks[7]; 

Bitboard rookMasks[64];
list<Bitboard> rookBlockers[64];
Magic rookMagics[64];

Bitboard bishopMasks[64];
list<Bitboard> bishopBlockers[64];
Magic bishopMagics[64];

Key rng()
{
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    return gen();
}

void generateRookMasks()
{
    for (int square = 0; square < 64; square++)
    {
        Bitboard mask = 0ULL;
        int rank = square / 8;
        int file = square % 8;

        for (int i = file + 1; i < 7; i++) // Right
        {
            setBit(&mask, rank * 8 + i);
        }
        for (int i = file - 1; i >= 1; i--) // Left
        {
            setBit(&mask, rank * 8 + i);
        }
        for (int i = rank + 1; i < 7; i++) // Down
        {
            setBit(&mask, i * 8 + file);
        }
        for (int i = rank - 1; i >= 1; i--) // Up
        {
            setBit(&mask, i * 8 + file);
        }
        rookMasks[square] = mask;
        // printBitboard(&mask);
    }
}

void generateBishopMasks()
{
    for (int square = 0; square < 64; square++)
    {
        Bitboard mask = 0ULL;
        int rank = indexToRank(square);
        int file = indexToFile(square);

        mask |= sendRayPre(&emptyBB, NE, square);
        mask |= sendRayPre(&emptyBB, SE, square);
        mask |= sendRayPre(&emptyBB, NW, square);
        mask |= sendRayPre(&emptyBB, SW, square);

        // & the mask with the edges of the board to save space
        mask &= ~edges;


        bishopMasks[square] = mask;
        //printBitboard(&mask);
    }
}

void generateRookBlockers()
{
    for (int square = 0; square < 64; square++)
    {
        Bitboard mask = rookMasks[square];
        int numBlockers = popCount(mask);

        for (int i = 0; i < pow(2, numBlockers); i++) // Loop through all possible blocker configurations
        {
            Bitboard blockers = 0ULL;

            // Set blockers
            for (int j = 0; j < numBlockers; j++)
            {
                if (i & (1 << j))
                {
                    setBit(&blockers, popLSB(&mask));
                }
                else
                    popLSB(&mask);
            }

            mask = rookMasks[square];

            rookBlockers[square].push_back(blockers);
        }
    }
}

void generateBishopBlockers()
{
    for (int square = 0; square < 64; square++)
    {
        Bitboard mask = bishopMasks[square];
        int numBlockers = popCount(mask);

        for (int i = 0; i < pow(2, numBlockers); i++) // Loop through all possible blocker configurations
        {
            Bitboard blockers = 0ULL;

            // Set blockers
            for (int j = 0; j < numBlockers; j++)
            {
                if (i & (1 << j))
                {
                    setBit(&blockers, popLSB(&mask));
                }
                else
                    popLSB(&mask);
            }

            mask = bishopMasks[square];

            bishopBlockers[square].push_back(blockers);
        }
    }
}

Magic *generateRookMagics()
{
    Magic *magics = new Magic[64];
    int epochs = 400;

    bool magicsFound[64] = {false};

    for (int i = 0; i < 64; i++)
    {
        magics[i].mask = rookMasks[i];
        magics[i].shift = 64 - popCount(magics[i].mask) - 4;
    }

    for (int i = 0; i < epochs; i++)
    {
        for (int square = 0; square < 64; square++)
        {
            unsigned int numTrys = 0;
            magics[square].shift++;
            while (true)
            {
                Key magic = rng();

                Bitboard table[8192] = {0};

                bool failed = false;
                for (Bitboard blockers : rookBlockers[square])
                {
                    Bitboard index = (blockers * magic) >> magics[square].shift;
                    if (table[index] != 0ULL)
                    {
                        failed = true;
                        break;
                    }
                    table[index] = 1ULL;
                }

                if (!failed)
                {
                    magicsFound[square] = true;
                    magics[square].magic = magic;
                    break;
                }

                numTrys++;
                if (numTrys > 100000)
                {
                    magics[square].shift--;
                    break;
                }
            }
        }
        int magicSize = 0;
        int magicsFoundCount = 0;
        for (int i = 0; i < 64; i++)
        {
            Magic magic = magics[i];
            magicSize += pow(2, 64 - magic.shift) * sizeof(Bitboard);
            if (magicsFound[i])
            {
                magicsFoundCount++;
            }
        }
        std::cout << "Epoch " << i << " size " << (int)magicSize / 1000 << "kb" << " Magics found: " << magicsFoundCount << std::endl;
    }

    // Write magics to file
    ofstream file;
    file.open("magics.txt");

    file << "Magic numbers for rooks\n";
    file << "\nMagic rookMagics[64] = {\n";

    for (int i = 0; i < 64; i++)
    {
        Magic magic = magics[i];
        file << "Magic(" << magic.mask << ", " << magic.magic << ", nullptr, " << (int)magic.shift << "),\n";
    }

    file << "};";

    file.close();

    return magics;
}

void generateBishopMagics()
{
    Magic *magics = new Magic[64];
    int epochs = 1000;

    bool magicsFound[64] = {false};

    for (int i = 0; i < 64; i++)
    {
        magics[i].mask = bishopMasks[i];
        magics[i].shift = 64 - popCount(magics[i].mask) - 4;
    }

    for (int i = 0; i < epochs; i++)
    {
        for (int square = 0; square < 64; square++)
        {
            unsigned int numTrys = 0;
            magics[square].shift++;
            while (true)
            {
                Key magic = rng();

                Bitboard table[4096] = {0};

                bool failed = false;
                for (Bitboard blockers : bishopBlockers[square])
                {
                    Bitboard index = (blockers * magic) >> magics[square].shift;
                    if (table[index] != 0ULL)
                    {
                        failed = true;
                        break;
                    }
                    table[index] = 1ULL;
                }

                if (!failed)
                {
                    magicsFound[square] = true;
                    magics[square].magic = magic;
                    break;
                }

                numTrys++;
                if (numTrys > 100000)
                {
                    magics[square].shift--;
                    break;
                }
            }
        }
        int magicSize = 0;
        int magicsFoundCount = 0;
        for (int i = 0; i < 64; i++)
        {
            Magic magic = magics[i];
            magicSize += pow(2, 64 - magic.shift) * sizeof(Bitboard);
            if (magicsFound[i])
            {
                magicsFoundCount++;
            }
        }
        std::cout << "Epoch " << i << " size " << (int)magicSize / 1000 << "kb" << " Magics found: " << magicsFoundCount << std::endl;
    }

    // Write magics to file
    ofstream file;
    file.open("magics.txt");

    file << "Magic numbers for bishops\n";
    file << "\nMagic bishopsMagics[64] = {\n";

    for (int i = 0; i < 64; i++)
    {
        Magic magic = magics[i];
        file << "Magic(" << magic.mask << ", " << magic.magic << ", nullptr, " << (int)magic.shift << "),\n";
    }

    file << "};";

    file.close();
}

void precomputeRookMoves()
{
    generateRookMasks();
    generateRookBlockers();
    for (int square = 0; square < 64; square++)
    {
        Magic magic = _rookMagicsPre[square];
        Bitboard mask = magic.mask;
        Key magicNumber = magic.magic;
        uint8_t shift = magic.shift;

        magic.table = new Bitboard[8192];

        // set all to 0
        for (int i = 0; i < 8192; i++)
        {
            magic.table[i] = 0;
        }

        for (Bitboard blockers : rookBlockers[square])
        {

            Key index = (blockers * magicNumber) >> shift;

            assert(magic.table[index] == 0);

            Bitboard moves = 0ULL;
            moves |= sendRayPre(&blockers, N, square);
            moves |= sendRayPre(&blockers, S, square);
            moves |= sendRayPre(&blockers, E, square);
            moves |= sendRayPre(&blockers, W, square);

            magic.table[index] = moves;
        }

        rookMagics[square] = magic;
    }
}

void precomputeBishopMoves()
{
    generateBishopMasks();
    generateBishopBlockers();
    for (int square = 0; square < 64; square++)
    {
        Magic magic = _bishopMagicsPre[square];
        Bitboard mask = magic.mask;
        Key magicNumber = magic.magic;
        uint8_t shift = magic.shift;

        magic.table = new Bitboard[8192];

        // set all to 0
        for (int i = 0; i < 8192; i++)
        {
            magic.table[i] = 0;
        }

        for (Bitboard blockers : bishopBlockers[square])
        {

            Key index = (blockers * magicNumber) >> shift;

            assert(magic.table[index] == 0);

            Bitboard moves = 0ULL;
            moves |= sendRayPre(&blockers, NE, square);
            moves |= sendRayPre(&blockers, SE, square);
            moves |= sendRayPre(&blockers, NW, square);
            moves |= sendRayPre(&blockers, SW, square);

            magic.table[index] = moves;
        }

        bishopMagics[square] = magic;
    }
}

// int main()
// {
//     initDirections();
//     initBBs();

//     generateBishopMasks();
//     generateBishopBlockers();
//     generateBishopMagics();
//     return 0;
// }
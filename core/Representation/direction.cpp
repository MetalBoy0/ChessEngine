#include "direction.h"

using namespace std;

Direction directions[64][64];
/*Distance to the edge of the board from every direction, in order:
  In order N S E W NE NW SE SW*/
uint8_t distToEdge[64][8];








// Init the direction array
void initDirections()
{
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            directions[i][j] = NULLDIR;
        }
    }
    for (int i = 0; i < 64; i++)
    {
        int rank = i / 8;
        int file = i % 8;
        for (int b = 0; b < 64; b++)
        {
            int rank2 = b / 8;
            int file2 = b % 8;
            if (i == b)
            {
                directions[i][b] = NULLDIR;
                continue;
            }
            if (rank == rank2 && file != file2)
            {
                directions[i][b] = file2 > file ? E : W;
            }
            else if (rank != rank2 && file == file2)
            {
                directions[i][b] = rank2 > rank ? N : S;
            }
            else if (rank2 - rank == file2 - file)
            {
                directions[i][b] = rank2 > rank ? (file2 > file ? NE : NW) : (file2 > file ? SE : SW);
            }
            else if (rank2 - rank == file - file2)
            {
                directions[i][b] = rank2 > rank ? (file2 > file ? NE : NW) : (file2 > file ? SE : SW);
            }
            else if (rank2 - rank == 2 && file2 - file == 1)
            {
                directions[i][b] = NNE;
            }
            else if (rank2 - rank == 1 && file2 - file == 2)
            {
                directions[i][b] = NEE;
            }
            else if (rank2 - rank == -1 && file2 - file == 2)
            {
                directions[i][b] = SEE;
            }
            else if (rank2 - rank == -2 && file2 - file == 1)
            {
                directions[i][b] = SSE;
            }
            else if (rank2 - rank == -2 && file2 - file == -1)
            {
                directions[i][b] = SSW;
            }
            else if (rank2 - rank == -1 && file2 - file == -2)
            {
                directions[i][b] = SWW;
            }
            else if (rank2 - rank == 1 && file2 - file == -2)
            {
                directions[i][b] = NWW;
            }
            else if (rank2 - rank == 2 && file2 - file == -1)
            {
                directions[i][b] = NNW;
            }
            else if (rank2 - rank == 1 && file2 - file == 0)
            {
                directions[i][b] = NN;
            }
            else if (rank2 - rank == -1 && file2 - file == 0)
            {
                directions[i][b] = SS;
            }
            else
            {
                directions[i][b] = NULLDIR;
            }
        }
        int n = 7 - rank;
        int s = rank;
        int w = file;
        int e = 7 - file;
        int ne = __builtin_fmin(n, e);
        int nw = __builtin_fmin(n, w);
        int se = __builtin_fmin(s, e);
        int sw = __builtin_fmin(s, w);
        distToEdge[i][0] = n;
        distToEdge[i][1] = s;
        distToEdge[i][2] = e;
        distToEdge[i][3] = w;
        distToEdge[i][4] = ne;
        distToEdge[i][5] = nw;
        distToEdge[i][6] = se;
        distToEdge[i][7] = sw;
    }
}
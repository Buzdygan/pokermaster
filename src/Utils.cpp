#include <algorithm>
#include <vector>
#include <cmath>
#include <fstream>
#include <cstdio>
#include <cstdarg>
#include <cstring>

#include "Utils.h"

const int CNUM = 52;

int HR[32487834];

int HR2[CNUM + 2][CNUM + 2];
const char* HR2_FILENAME = "../data/hr2.dat";

const int DEBUG_LEVEL = 1;

int iabs(int x)
{
    return x > 0 ? x : -x;
}

int log(int level, const char* format, ...)
{
    if (level >= DEBUG_LEVEL)
        return 0;
    va_list vl;
    va_start(vl, format);
    vprintf(format, vl);
    va_end(vl);
}

int other(int player_number)
{
    return (player_number + 1) & 1;
}

void printCard(int card)
{
    char F[13] = {'2', '3', '4', '5', '6', '7', '8', '9', 'T', 'W', 'D', 'K', 'A'};
    char C[4] = {'c', 'd', 'h', 's'};
    card --;
    printf("%c%c ", F[card / 4], C[card % 4]);
}

void printHand(vector<int> cards)
{
    for (int i = 0; i < cards.size(); i++)
    {
        printCard(cards[i]);
        printf(" ");
    }
}

HandEvaluator::HandEvaluator()
{
	memset(HR, 0, sizeof(HR));
	FILE * fin = fopen("../data/HandRanks.dat", "rb");
	size_t bytesread = fread(HR, sizeof(HR), 1, fin);	// get the HandRank Array
	fclose(fin);
    if(!_loadHR2())
    {
        _computeHR2();
        _saveHR2();
    }
}

void HandEvaluator::_computeHR2()
{
    const double DENOM = 19600.0;
    for (int i = 1; i <= CNUM - 1; i++)
        for (int j = i + 1; j <= CNUM; j++)
            {
                double sum = 0.0;
                for (int c1 = 1; c1 <= CNUM - 2; c1++)
                    if (c1 != i && c1 != j)
                        for (int c2 = c1 + 1; c2 <= CNUM - 1; c2++)
                            if (c2 != i && c2 != j)
                                for (int c3 = c2 + 1; c3 <= CNUM; c3++)
                                    if (c3 != i && c3 != j)
                                        sum += evaluateHand(i, j, c1, c2, c3) / DENOM;

                HR2[i][j] = HR2[j][i] = int(sum);
            }
}

void HandEvaluator::_saveHR2()
{
    FILE * fout = fopen(HR2_FILENAME, "w");
    for (int i = 1; i <= CNUM; i++)
        for (int j = 1; j <= CNUM; j++)
            if (i != j)
                fprintf(fout, "%d\n", HR2[i][j]);
    fclose(fout);
}

bool HandEvaluator::_loadHR2()
{
    try
    {
        FILE* fin = fopen(HR2_FILENAME, "r");
        if (fin == NULL)
            return false;
        for (int i = 1; i <= CNUM; i++)
            for (int j = 1; j <= CNUM; j++)
                if (i != j)
                    fscanf(fin, "%d\n", &HR2[i][j]);
        fclose(fin);
        return true;
    }
    catch(int e)
    {
        return false;
    }
}

int HandEvaluator::evaluateHand(vector<int> cards)
{
    int p = HR[53 + cards[0]];
    for (int i = 1; i < cards.size(); i++)
        p = HR[p + cards[i]];
    if (cards.size() < 7)
        return HR[p];
    else
        return p;
}

int HandEvaluator::evaluateHand(int c1, int c2, int c3, int c4, int c5, int c6, int c7)
{
    if (c3 > 0)
    {
        int p = HR[53 + c1];
        p = HR[p + c2];
        p = HR[p + c3];
        p = HR[p + c4];
        p = HR[p + c5];
        if (c6 > 0)
            p = HR[p + c6];
        if (c7 > 0)
            return HR[p + c7];
        return HR[p];
    }
    else
        return HR2[c1][c2];
}


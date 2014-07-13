#include<algorithm>
#include<vector>
#include <cmath>

#include "Utils.h"

int HR[32487834];

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
    {
        int f1 = (c1 - 1) / 4, f2 = (c2 - 1) / 4;
        int diff = max(0, 3 - iabs(f1 - f2));
        return diff * 200 + max(f1, f2) * 13 + min(f1, f2);
    }
}


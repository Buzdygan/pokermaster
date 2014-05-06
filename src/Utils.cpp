#include<algorithm>
#include<vector>

#include "Utils.h"

int HR[32487834];

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
    int ind = 0;
    int p = HR[53 + cards[ind++]];
    p = HR[p + cards[ind++]];
    p = HR[p + cards[ind++]];
    p = HR[p + cards[ind++]];
    p = HR[p + cards[ind++]];
    p = HR[p + cards[ind++]];
    return HR[p + cards[ind++]];
}

#include<algorithm>
#include<vector>

#include "Utils.h"

int other(int player_number)
{
    return (player_number + 1) & 1;
}

HandEvaluator::HandEvaluator()
{
}

/* Returns score between 0 and 5 */
int HandEvaluator::evaluateHand(vector<int> cards)
{
    int c0 = (cards[0] + 1) / 2, c1 = (cards[1] + 1) / 2;
    if (c0 == c1)
        return 2 + c0;
    return c0 + c1 - 3;

}

void printCard(int card)
{
    printf("%d", card);
}

void printHand(vector<int> cards)
{
    for (int i = 0; i < cards.size(); i++)
    {
        printCard(cards[i]);
        printf(" ");
    }
}

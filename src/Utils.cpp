#include<algorithm>
#include<vector>

#include "Utils.h"

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

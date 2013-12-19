#include <utility>
#include <cstdio>
#include <cmath>
#include <vector>
#include "HumanPlayer.h"
#include "Utils.h"

using namespace std;

HumanPlayer::HumanPlayer(int p_number, int p_cash)
{
    number = p_number;
    cash = p_cash;
}
/* starts new round with two dealt cards */
void HumanPlayer::startNewRound(pair<int, int> cards)
{
    printf("STARTING NEW ROUND WITH CARDS: ");
    printCard(cards.first);
    printf(", ");
    printCard(cards.second);
    printf("\n");
}

/* starts new phase with new cards */
void HumanPlayer::startNewPhase(vector<int> cards)
{
    printf("NEXT PHASE: ");
    for (int c = 0; c < cards.size(); c++)
        printCard(cards[c]);
}

/* learns opponent cards */
void HumanPlayer::showOpponentCards(pair<int, int> cards)
{
    printf("OPPONENTS CARDS: ");
    printCard(cards.first);
    printf(", ");
    printCard(cards.second);
    printf("\n");
}

/* gets bet in this phase, given opponent's bet. Bet -1 signifies start of the phase */
int HumanPlayer::getBet(int opponent_bet)
{
    printf("OPPONENT BET: %d\nWRITE YOURS\n", opponent_bet);
    int bet;
    scanf("%d", &bet);
    return bet;
}
/* gives info on who won the round with what stake */
void HumanPlayer::announceRoundWinner(int winner, int stake)
{
    if (winner == other(number))
        cash -= stake;
    if (winner == number)
        cash += stake;
    printf("DEBUG: human player %d, cash: %d\n", number, cash);
}

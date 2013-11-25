#include <utility>
#include <cstdio>
#include <cmath>
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
}

/* learns opponent cards */
void HumanPlayer::showOpponentCards(pair<int, int> cards)
{
}

/* deals one table card */
void HumanPlayer::showTableCard(int card)
{
}
/* gets bet in this phase, given opponent's bet. Bet -1 signifies start of the phase */
int HumanPlayer::getBet(int opponent_bet)
{
    return min(opponent_bet, cash);
}
/* gives info on who won the round with what stake */
void HumanPlayer::announceRoundWinner(int winner, int stake)
{
    if (winner == other(number))
        cash -= stake;
    if (winner == number)
        cash += stake;
    printf("DEBUG: player %d, announceRoundWinner, cash: %d\n", number, cash);
}

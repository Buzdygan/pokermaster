#include <utility>
#include "DummyPlayer.h"


using namespace std;



DummyPlayer::DummyPlayer(int p_number, int p_cash)
{
    number = p_number;
    cash = p_cash;
}
/* starts new round with two dealt cards */
void DummyPlayer::startNewRound(pair<int, int> cards)
{
}
/* deals one table card */
void DummyPlayer::showTableCard(int card)
{
}
/* gets bet in this phase, given opponent's bet. Bet -1 signifies start of the phase */ 
int DummyPlayer::getBet(int opponent_bet)
{
    return 0;
}
/* gives info on who won the round with what stake */
void DummyPlayer::announceRoundWinner(int winner, int stake)
{
}

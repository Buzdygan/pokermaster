
#include <vector>
#include "CfrPlayer.h"


using namespace std;

CfrPlayer::CfrPlayer(int p_number, int p_cash, int buckets_number, int max_stake)
{

}

/* starts new round with two dealt cards */
void CfrPlayer::startNewRound(pair<int, int> cards)
{

}

/* starts new phase with new cards */
void CfrPlayer::startNewPhase(vector<int> cards)
{
}

/* learns opponent cards */
void CfrPlayer::showOpponentCards(pair<int, int> cards)
{

}

/* gets bet in this phase, given opponent's bet. Bet -1 signifies start of the phase */
int CfrPlayer::getBet(int opponent_bet)
{
    return opponent_bet;

}

/* gives info on who won the round with what stake */
void CfrPlayer::announceRoundWinner(int winner, int stake)
{

}

void CfrPlayer::initFromFile(char* filename)
{

}

void CfrPlayer::saveToFile(char* filename)
{

}

void CfrPlayer::init()
{

}

#ifndef DUMMY_PLAYER_H
#define DUMMY_PLAYER_H

#include "Player.h"
using namespace std;

class DummyPlayer : public Player
{
    public:
        DummyPlayer(int p_number, int p_cash);
        /* starts new round with two dealt cards */
        void startNewRound(pair<int, int> cards);
        /* deals one table card */
        void showTableCard(int card);
        /* gets bet in this phase, given opponent's bet. Bet -1 signifies start of the phase */ 
        int getBet(int opponent_bet);
        /* gives info on who won the round with what stake */
        void announceRoundWinner(int winner, int stake);
    private:
        int number, cash;
};

#endif

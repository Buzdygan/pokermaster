#ifndef PLAYER_H
#define PLAYER_H

#include <cstdlib>
#include <utility>
using namespace std;

class Player
{
    public:
        /* starts new round with two dealt cards */
        virtual void startNewRound(pair<int, int> cards) = 0;
        /* learns opponent cards */
        virtual void showOpponentCards(pair<int, int> cards) = 0;
        /* deals one table card */
        virtual void showTableCard(int card) = 0;
        /* gets bet in this phase, given opponent's bet. Bet -1 signifies start of the phase */ 
        virtual int getBet(int opponent_bet) = 0;
        /* gives info on who won the round with what stake */
        virtual void announceRoundWinner(int winner, int stake) = 0;
        virtual ~Player ();

    protected:
        Player() {}
};


#endif

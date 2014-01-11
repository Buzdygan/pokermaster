#ifndef PLAYER_H
#define PLAYER_H

#include <cstdlib>
#include <utility>
#include <vector>
using namespace std;

class Player
{
    public:
        /* starts new round */
        virtual void startNewRound() = 0;
        /* Annotates random action */
        virtual void annotateRandomAction(int action_id) = 0;
        /* annotates opponent's action */
        virtual void annotateOpponentAction(int action_id) = 0;
        /* Get player's action */
        virtual int getAction(int information_set_id, vector<int> available_actions) = 0;
        /* gives info on who won the round with what stake */
        virtual void endRound(double cash_change) = 0;
        virtual ~Player ();

    protected:
        Player() {}
};


#endif

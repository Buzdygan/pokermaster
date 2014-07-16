#ifndef DUMMY_PLAYER_H
#define DUMMY_PLAYER_H

#include "Player.h"
#include "GameAbstraction.h"
using namespace std;

class DummyPlayer : public Player
{
    public:
        DummyPlayer();
        /* Annotates random action */
        void annotateRandomAction(int action_id);
        /* annotates this player's action */
        void annotatePlayerAction(int action_id);
        /* annotates opponent's action */
        void annotateOpponentAction(int action_id);
        /* Get player's action */
        int getAction(vector<int> available_actions);
        /* gives info on who won the round with what stake */
        void endRound(double cash_change);
    private:
        GameAbstraction* game;
};

#endif

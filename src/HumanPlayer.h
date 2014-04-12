#ifndef HUMAN_PLAYER_H
#define HUMAN_PLAYER_H

#include "Player.h"
#include "GameAbstraction.h"
using namespace std;

class HumanPlayer : public Player
{
    public:
        HumanPlayer();
        /* starts new round */
        void startNewRound();
        /* Annotates random action */
        void annotateRandomAction(int action_id);
        /* annotates opponent's action */
        void annotateOpponentAction(int action_id);
        /* Get player's action */
        int getAction(int information_set_id, vector<int> available_actions);
        /* gives info on who won the round with what stake */
        void endRound(double cash_change);
    private:
        GameAbstraction* game;
};

#endif

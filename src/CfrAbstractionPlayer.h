#ifndef CFR_ABSTRACTION_PLAYER_H
#define CFR_ABSTRACTION_PLAYER_H

#include "Player.h"
#include "Cfr.h"
#include "GameAbstraction.h"
#include "HoldemPokerAbstraction.h"
using namespace std;

class CfrAbstractionPlayer : public Player
{
    public:
        CfrAbstractionPlayer(int player_num,
                  Cfr* strategy,
                  HoldemPokerAbstraction* game);
        /* Annotates random action */
        virtual void annotateRandomAction(int action_id);
        /* annotates this player's action */
        void annotatePlayerAction(int action_id);
        /* annotates opponent's action */
        void annotateOpponentAction(int action_id);
        /* Get player's action */
        virtual int getAction(vector<int> available_actions);
        /* gives info on who won the round with what stake */
        void endRound(double cash_change);
    protected:
        int CARDS_FOR_PHASE[4];
        int cur_stake;
        int player_num;
        int random_phase;
        bool first_action;
        int bids_number;
        vector<int> all_cards;

        Cfr* strategy;
        HoldemPokerAbstraction* game;
};

#endif

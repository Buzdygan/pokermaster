#ifndef CFR_PLAYER_H
#define CFR_PLAYER_H

#include "Player.h"
#include "Cfr.h"
#include "GameAbstraction.h"
#include "HoldemPokerAbstraction.h"
using namespace std;

class CfrPlayer : public Player
{
    public:
        CfrPlayer(int player_num,
                  Cfr* strategy,
                  HoldemPokerAbstraction* game);
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
        void _logBid(int bid);
    private:
        int cur_stake;
        int player_num;
        int random_phase;
        bool first_action;
        int bids_number;
        vector<int> new_cards;
        Cfr* strategy;
        HoldemPokerAbstraction* game;
};

#endif

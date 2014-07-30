#ifndef CFR_MOD_PLAYER_H
#define CFR_MOD_PLAYER_H

#include "Player.h"
#include "CfrPlayer.h"
#include "ModCfr.h"
#include "BasketManager.h"
#include "GameAbstraction.h"
#include "HoldemPokerModAbstraction.h"

using namespace std;

class CfrModPlayer : public Player
{
    public:
        CfrModPlayer(int player_num,
                  ModCfr* strategy,
                  HoldemPokerModAbstraction* game);
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
        int _logBid(int bid);
    protected:
        dist prev_opp_dist;

        int CARDS_FOR_PHASE[4];
        int cur_stake;
        int player_num;
        int random_phase;
        bool first_action;
        int bids_number;
        vector<int> all_cards;

        ModCfr* strategy;
        HoldemPokerModAbstraction* game;
};

#endif

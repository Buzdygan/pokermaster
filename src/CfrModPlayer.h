#ifndef CFR_MOD_PLAYER_H
#define CFR_MOD_PLAYER_H

#include "Player.h"
#include "CfrPlayer.h"
#include "Cfr.h"
#include "BasketManager.h"
#include "GameAbstraction.h"
#include "HoldemPokerModAbstraction.h"

using namespace std;

class CfrModPlayer : public CfrPlayer
{
    public:
        CfrModPlayer(int player_num,
                  Cfr* strategy,
                  HoldemPokerModAbstraction* game);
        /* Annotates random action */
        void annotateRandomAction(int action_id);
        /* Get player's action */
        int getAction(vector<int> available_actions);
    protected:
        vector<int> new_cards;
        vector<int> indexes;
        dist prev_opp_dist;
};

#endif

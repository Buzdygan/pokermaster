#ifndef __HOLDEM_POKER_MOD_ABSTRACTION_H
#define __HOLDEM_POKER_MOD_ABSTRACTION_H

#include "HoldemPokerAbstraction.h"
#include "BasketManager.h"

class HoldemPokerModAbstraction : public HoldemPokerAbstraction
{
    public:
        HoldemPokerModAbstraction(BasketManager* manager);
        ~HoldemPokerModAbstraction();
        int getStateId();
        int getISetId();
        vector<int> getActionIds();
        vector<int> getActionIds(int bids_num);
        dist getInformationSetIds(dist baskets);
};

#endif

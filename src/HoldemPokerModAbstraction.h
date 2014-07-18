#ifndef __HOLDEM_POKER_MOD_ABSTRACTION_H
#define __HOLDEM_POKER_MOD_ABSTRACTION_H

#include "HoldemPokerAbstraction.h"
#include "BasketManager.h"

class HoldemPokerModAbstraction : public HoldemPokerAbstraction
{
    public:
        HoldemPokerModAbstraction(BasketManager* manager);
        ~HoldemPokerModAbstraction();
        int getInformationSetId();
        dist getInformationSetIds(dist baskets);
};

#endif
#include "HoldemPokerAbstraction.h"
#include "HoldemPokerModAbstraction.h"
#include "BasketManager.h"

using namespace std;

HoldemPokerModAbstraction::HoldemPokerModAbstraction(BasketManager* mng) : HoldemPokerAbstraction(mng)
{
}

HoldemPokerModAbstraction::~HoldemPokerModAbstraction()
{
}

int HoldemPokerModAbstraction::getInformationSetId()
{
    int res = 0;
    int mult = 1;

    res += cur_player * mult;
    mult *= 2;

    res += bidding_phase * mult;
    mult *= 4;

    res += player_basket[cur_player] * mult;
    mult *= phase_actions[random_phase];

    res += player_basket[other(cur_player)] * mult;
    mult *= phase_actions[random_phase];

    res += bids_number * mult;
    mult *= 3;

    res += cur_stake * mult;
    mult *= 20;

    res += agreed_stake * mult;
    mult *= 20;

    return res;
}


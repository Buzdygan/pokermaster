#include <map>
#include "HoldemPokerAbstraction.h"
#include "HoldemPokerModAbstraction.h"
#include "BasketManager.h"


using namespace std;

map<int, vector<int> > imap;
bool printed = false;

HoldemPokerModAbstraction::HoldemPokerModAbstraction(BasketManager* mng) : HoldemPokerAbstraction(mng)
{
}

HoldemPokerModAbstraction::~HoldemPokerModAbstraction()
{
}

vector<int> HoldemPokerModAbstraction::getActionIds(int bids_num)
{
    vector<int> action_ids;
    if (cur_stake > agreed_stake)
        action_ids.push_back(ACTION_FOLD);
    action_ids.push_back(ACTION_CALL);
    if (bids_num < MAX_BIDS_NUMBER && cur_stake < LOG_MAX_STAKE)
    {
        action_ids.push_back(ACTION_RAISE);
        action_ids.push_back(ACTION_ALL_IN);
    }
    return action_ids;
}

int HoldemPokerModAbstraction::getISetId()
{
    int res = 0;
    int mult = 1;
    int ind = cur_player == 2 ? cur_player - 2 : cur_player;

    res += cur_player * mult;
    mult *= 3;

    res += bidding_phase * mult;
    mult *= 5;

    res += player_basket[ind] * mult;
    mult *= MAX_BASKETS_NUMBER;

    res += bids_number * mult;
    mult *= 3;

    res += cur_stake * mult;
    mult *= 9;

    res += agreed_stake * mult;

    return res;


}

int HoldemPokerModAbstraction::getStateId()
{
    int res = 0;
    int mult = 1;
    int ind = cur_player == 2 ? cur_player - 2 : cur_player;

    res += cur_player * mult;
    mult *= 3;

    res += bidding_phase * mult;
    mult *= 5;

    res += player_basket[ind] * mult;
    mult *= MAX_BASKETS_NUMBER;

    res += player_basket[other(ind)] * mult;
    mult *= MAX_BASKETS_NUMBER;

    res += bids_number * mult;
    mult *= 3;

    res += cur_stake * mult;
    mult *= 9;

    res += agreed_stake * mult;

    return res;


}

dist HoldemPokerModAbstraction::getInformationSetIds(dist baskets)
{
    int res = 0;
    int mult = 1;
    int backup_mult = 1;
    int ind = cur_player == 2 ? cur_player - 2 : cur_player;

    res += cur_player * mult;
    mult *= 3;

    res += bidding_phase * mult;
    mult *= 5;

    res += player_basket[ind] * mult;
    mult *= MAX_BASKETS_NUMBER;

    //res += player_basket[other(cur_player)] * mult;
    backup_mult = mult;
    mult *= MAX_BASKETS_NUMBER;

    res += bids_number * mult;
    mult *= 3;

    res += cur_stake * mult;
    mult *= 9;

    res += agreed_stake * mult;

    dist information_sets;
    for (int i = 0; i < baskets.size(); i ++)
        information_sets.push_back(make_pair(res + backup_mult * baskets[i].first, baskets[i].second));
    return information_sets;
}


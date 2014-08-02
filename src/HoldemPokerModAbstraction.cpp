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

int HoldemPokerModAbstraction::getInformationSetId()
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

    /*
    vector<int> v;
    v.push_back(cur_player);
    v.push_back(bidding_phase);
    v.push_back(player_basket[cur_player]);
    v.push_back(player_basket[other(cur_player)]);
    v.push_back(bids_number);
    v.push_back(cur_stake);
    v.push_back(agreed_stake);
    if (!imap.count(res))
        imap[res] = v;
    else
    {
        vector<int> v2 = imap[res];
        for (int i = 0; i < 7; i++)
            if (v[i] != v2[i])
            {
                printf("ERROR at %d\n", res);
                for (int j = 0; j < 7; j++)
                    printf("%d vs %d\n", v[j], v2[j]);
                break;
            }
    }
    if (!printed)
        printf("imap size: %d\n", (int)imap.size());
    if (!printed && imap.size() >= 1347)
    {
        for (map<int, vector<int> >::iterator it = imap.begin(); it != imap.end(); it++)
        {
            int is_id = it -> first;
            vector<int> w = it -> second;
            printf("%d:", is_id);
            for (int j = 0; j < 7; j++)
                printf(" %d", w[j]);
            printf("\n");
        }
        printed = true;
    }
    */

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


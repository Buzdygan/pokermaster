#include <vector>
#include <cstdio>
#include "CfrPlayer.h"
#include "CfrModPlayer.h"
#include "BasketManager.h"
#include "HoldemPokerModAbstraction.h"

using namespace std;

CfrModPlayer::CfrModPlayer(int pnum,
                  ModCfr* stg,
                  HoldemPokerModAbstraction* gam)
{
    CARDS_FOR_PHASE[0] = 2;
    CARDS_FOR_PHASE[1] = 5;
    CARDS_FOR_PHASE[2] = 6;
    CARDS_FOR_PHASE[3] = 7;
    random_phase = 0;
    bids_number = 1;
    cur_stake = 1;
    player_num = pnum;
    strategy = stg;
    game = gam;
}
/* Annotates random action */
void CfrModPlayer::annotateRandomAction(int card_id)
{
    all_cards.push_back(card_id);
    /* We end the random phase */
    if (all_cards.size() == CARDS_FOR_PHASE[random_phase])
    {
        int bnum = game -> getBasketManager() -> getBasket(all_cards);
        printf("Player%d basket: %d\n", player_num, bnum);
        prev_opp_dist = game -> getBasketManager() -> getOpponentBasketDist(random_phase, all_cards);
        int action_id;
        if (player_num == 0)
            action_id = encode_basket_pair(bnum, 0);
        else
            action_id = encode_basket_pair(0, bnum);
        game -> makeAction(action_id);
        random_phase ++;
    }

}

/* annotates this player's action */
void CfrModPlayer::annotatePlayerAction(int action_id)
{
    game -> makeAction(_logBid(action_id));
}

/* annotates opponent's action */
void CfrModPlayer::annotateOpponentAction(int action_id)
{
    game -> makeAction(_logBid(action_id));
}

/* Get player's action */
int CfrModPlayer::getAction(vector<int> available_actions)
{
    dist isets_dist = game -> getInformationSetIds(prev_opp_dist);
    printf("Player%d making decision\n", player_num);
    int action_id = strategy -> getActionId(isets_dist, game -> getActionIds(bids_number));
    if (action_id == game -> ACTION_FOLD)
        return 0;
    if (action_id == game -> ACTION_CALL)
        return cur_stake;
    if (action_id == game -> ACTION_RAISE)
        return cur_stake * 2;
    return cur_stake;

}

/* gives info on who won the round with what stake */
void CfrModPlayer::endRound(double cash_change)
{
}

int CfrModPlayer::_logBid(int bid)
{
    int action_id = game -> ACTION_FOLD;
    if (bid == cur_stake)
        action_id = game -> ACTION_CALL;
    if (bid > cur_stake)
        action_id = game -> ACTION_RAISE;
    cur_stake = bid;
    return action_id;
}

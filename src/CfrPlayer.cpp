#include <vector>
#include <cstdio>
#include "CfrPlayer.h"
#include "BasketManager.h"

using namespace std;

int CARDS_FOR_PHASE[4] = {2,5,6,7};

CfrPlayer::CfrPlayer(int pnum, Cfr* stg, HoldemPokerAbstraction* g)
{
    random_phase = 0;
    bids_number = 1;
    cur_stake = 1;
    prev_opponent_basket = -1;
    player_num = pnum;
    strategy = stg;
    game = g;
}

/* Annotates dealt card */
void CfrPlayer::annotateRandomAction(int card_id)
{
    all_cards.push_back(card_id);
    new_cards.push_back(card_id);
    /* We end the random phase */
    //TODO źle źle, musisz uwzględniać rozkład koszyków przeciwnika
    if (all_cards.size() == CARDS_FOR_PHASE[random_phase])
    {
        int bnum = game -> getBasketManager() -> getBasket(all_cards);
        printf("Player%d basket: %d\n", player_num, bnum);
        int action_id;
        if (player_num == 0)
            action_id = encode_basket_pair(bnum, 0);
        else
            action_id = encode_basket_pair(0, bnum);
        game -> makeAction(action_id);
        random_phase ++;
        new_cards.clear();
    }
}

/* annotates this player's action */
void CfrPlayer::annotatePlayerAction(int action_id)
{
    game -> makeAction(_logBid(action_id));
}

/* annotates opponent's action */
void CfrPlayer::annotateOpponentAction(int action_id)
{
    game -> makeAction(_logBid(action_id));
}

/* Get player's action */
int CfrPlayer::getAction(vector<int> available_actions)
{
    printf("Player%d making decision\n", player_num);
    int action_id = strategy -> getActionId(game -> getInformationSetId(),
                                            game -> getActionIds(bids_number));
    if (action_id == game -> ACTION_FOLD)
        return 0;
    if (action_id == game -> ACTION_CALL)
        return cur_stake;
    if (action_id == game -> ACTION_RAISE)
        return cur_stake * 2;
    return cur_stake;
}

/* gives info on who won the round with what stake */
void CfrPlayer::endRound(double cash_change)
{
}

int CfrPlayer::_logBid(int bid)
{
    int action_id = game -> ACTION_FOLD;
    if (bid == cur_stake)
        action_id = game -> ACTION_CALL;
    if (bid > cur_stake)
        action_id = game -> ACTION_RAISE;
    cur_stake = bid;
    return action_id;
}

#include <vector>
#include "CfrPlayer.h"

using namespace std;

int CARDS_FOR_PHASE[4] = {2,3,1,1};

CfrPlayer::CfrPlayer(int pnum, Cfr* stg, HoldemPokerAbstraction* g)
{
    random_phase = 0;
    bids_number = 0;
    cur_stake = 1;
    player_num = pnum;
    strategy = stg;
    game = g;
    first_action = true;
}

/* Annotates dealt card */
void CfrPlayer::annotateRandomAction(int card_id)
{
    new_cards.push_back(card_id);
    /* We end the random phase */
    if (new_cards.size() == CARDS_FOR_PHASE[random_phase])
    {
        game -> logCards(player_num, new_cards, random_phase);
        new_cards.clear();
        bids_number = 0;
        random_phase ++;
    }
}

/* annotates this player's action */
void CfrPlayer::annotatePlayerAction(int action_id)
{
    _logBid(action_id);
}

/* annotates opponent's action */
void CfrPlayer::annotateOpponentAction(int action_id)
{
    _logBid(action_id);
}

/* Get player's action */
int CfrPlayer::getAction(vector<int> available_actions)
{
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

void CfrPlayer::_logBid(int bid)
{
    if (first_action)
        first_action = false;
    else
    {
        int action_id = game -> ACTION_FOLD;
        if (bid == cur_stake)
            action_id = game -> ACTION_CALL;
        if (bid > cur_stake)
            action_id = game -> ACTION_RAISE;
        cur_stake = bid;
        game -> logAction(player_num, bid, 4 + bids_number);
    }
    bids_number ++;
}

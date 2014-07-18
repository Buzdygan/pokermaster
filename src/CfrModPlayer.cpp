#include <vector>
#include <cstdio>
#include "CfrPlayer.h"
#include "CfrModPlayer.h"
#include "BasketManager.h"
#include "HoldemPokerModAbstraction.h"

using namespace std;

CfrModPlayer::CfrModPlayer(int player_num,
                  Cfr* strategy,
                  HoldemPokerModAbstraction* gam) : CfrPlayer(player_num, strategy, (HoldemPokerAbstraction*) gam)
{
}
/* Annotates random action */
void CfrModPlayer::annotateRandomAction(int card_id)
{
    all_cards.push_back(card_id);
    new_cards.push_back(card_id);
    /* We end the random phase */
    //TODO źle źle, musisz uwzględniać rozkład koszyków przeciwnika
    if (all_cards.size() == CARDS_FOR_PHASE[random_phase])
    {
        indexes.push_back(game -> getBasketManager() -> getIndex(random_phase, new_cards));
        int bnum = game -> getBasketManager() -> getBasket(all_cards);
        prev_opp_dist = game -> getBasketManager() -> getOpponentBasketDist(random_phase, indexes);
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

/* Get player's action */
int CfrModPlayer::getAction(vector<int> available_actions)
{
    HoldemPokerModAbstraction* mod_game = (HoldemPokerModAbstraction*) game;
    dist isets_dist = mod_game -> getInformationSetIds(prev_opp_dist);
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


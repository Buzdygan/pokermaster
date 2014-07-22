#include <vector>
#include <cstdio>
#include "CfrAbstractionPlayer.h"
#include "BasketManager.h"
#include "Utils.h"

using namespace std;


CfrAbstractionPlayer::CfrAbstractionPlayer(int pnum, Cfr* stg, HoldemPokerAbstraction* g)
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
    game = g;
}

/* Annotates dealt card */
void CfrAbstractionPlayer::annotateRandomAction(int action_id)
{
    printBaskets(action_id);
    game -> makeAction(action_id);
}

/* annotates this player's action */
void CfrAbstractionPlayer::annotatePlayerAction(int action_id)
{
    game -> makeAction(action_id);
}

/* annotates opponent's action */
void CfrAbstractionPlayer::annotateOpponentAction(int action_id)
{
    game -> makeAction(action_id);
}

/* Get player's action */
int CfrAbstractionPlayer::getAction(vector<int> available_actions)
{
    printf("Player%d making decision\n", player_num);
    return strategy -> getActionId(game -> getInformationSetId(),
                                            game -> getActionIds(bids_number));
}

/* gives info on who won the round with what stake */
void CfrAbstractionPlayer::endRound(double cash_change)
{
}

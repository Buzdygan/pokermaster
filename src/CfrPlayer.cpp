#include <vector>
#include "CfrPlayer.h"

using namespace std;

CfrPlayer::CfrPlayer(Cfr* stg)
{
    strategy = stg;
}
/* starts new round */
void CfrPlayer::startNewRound()
{

}
/* Annotates random action */
void CfrPlayer::annotateRandomAction(int action_id)
{

}
/* annotates opponent's action */
void CfrPlayer::annotateOpponentAction(int action_id)
{

}
/* Get player's action */
int CfrPlayer::getAction(int information_set_id, vector<int> available_actions)
{
    return strategy -> getActionId(information_set_id, available_actions);
}
/* gives info on who won the round with what stake */
void CfrPlayer::endRound(double cash_change)
{

}

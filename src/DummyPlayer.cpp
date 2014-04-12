#include <utility>
#include <cstdio>
#include <cmath>
#include <vector>
#include "DummyPlayer.h"

using namespace std;

DummyPlayer::DummyPlayer()
{
}

/* starts new round */
void DummyPlayer::startNewRound()
{

}

/* Annotates random action */
void DummyPlayer::annotateRandomAction(int action_id)
{
}

/* annotates opponent's action */
void DummyPlayer::annotateOpponentAction(int action_id)
{
}

/* Get player's action */
int DummyPlayer::getAction(int information_set_id, vector<int> available_actions)
{
    if (available_actions.size() > 1)
        return available_actions[1];
    else
        return available_actions[0];
}

/* gives info on who won the round with what stake */
void DummyPlayer::endRound(double cash_change)
{

}

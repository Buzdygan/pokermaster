#include <ctime>
#include <cstdlib>
#include "SimplePoker.h"

SimplePoker::SimplePoker()
{
    start_player = rand() % 2;
    cur_player = RANDOM_PLAYER_NR;
    cur_stake = agreed_stake = 0;
    random_phase = 0;
    for (int c = 1; c <= CARDS_NUMBER; c++)
        deck.push_back(c);
}

SimplePoker::~SimplePoker()
{
}

int SimplePoker::getInformationSetId()
{
    return -1;
}

utility SimplePoker::getUtility()
{
    return make_pair(-1.0, -1.0);
}

int SimplePoker::getPlayerId()
{
    return cur_player;
}

dist SimplePoker::getActionDistribution()
{
    dist d;
    double prob = 1.0 / deck.size();
    for (int i = 0; i < (int) deck.size(); i++)
        d.push_back(make_pair(deck[i], prob));
    return d;
}

/* returns player that sees the random action.
 * -1 means all players */
int SimplePoker::randomActionPlayer()
{
    if (random_phase < FINAL_RANDOM_PHASE)
        return random_phase;
    return ALL_PLAYERS;
}

bool SimplePoker::isFinal()
{
    return false;
}
vector<int> SimplePoker::getActionIds()
{
    return vector<int> ();
}

void SimplePoker::endGame(int winner)
{
}

void SimplePoker::makeAction(int action_id)
{
    /* random player, action_id = card id */
    if (cur_player == RANDOM_PLAYER_NR)
    {
        /* card for single player */
        if (random_phase < FINAL_RANDOM_PHASE)
            player_cards[random_phase].push_back(action_id);
        /* card for both players */
        else
        {
            player_cards[0].push_back(action_id);
            player_cards[1].push_back(action_id);
        }
        random_phase ++;
        if (random_phase == FINAL_RANDOM_PHASE)
        {
            cur_player = start_player;
        }
    }
    else
    {
        /* player looses */
        if (action_id < cur_stake)
            endGame(other(cur_player));
        if (action_id == cur_stake)
        {

        }

    }
}
void SimplePoker::unmakeAction(int action_id)
{

}

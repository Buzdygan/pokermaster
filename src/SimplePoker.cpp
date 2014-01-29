#include <ctime>
#include <cstdlib>
#include "SimplePoker.h"
#include "Utils.h"


SimplePoker::SimplePoker()
{
    start_player = rand() % 2;
    cur_player = RANDOM_PLAYER_NR;
    cur_stake = agreed_stake = 1;
    bidding_phase = 0;
    bids_number = 0;
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

/* returns player that sees the random action.*/
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
    /* if it's random player turn, action_id = dealt card id
     * otherwise it's the bet of a player. First player has to bid at least 1. */
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
            cur_player = start_player;
        }
        random_phase ++;
    }
    else
    {
        bids_number ++;
        int bet = action_id;
        if (bet > MAX_STAKE)
        {
            fprintf(stderr, "ERROR: Betting %d: more than MAX_STAKE=%d\n", bet, MAX_STAKE);
            throw 1;
        }
        /* player looses */
        else if (bet < cur_stake)
        {
            endGame(other(cur_player));
        }
        else if (bet == cur_stake)
        {
            /* Second player agrees */
            if (bids_number >= 2)
                _endOfBiddingPhase();
            else
                cur_player = other(cur_player);
        }
        if (bet > cur_stake)
        {
            if (bids_number >= 3)
            {
                fprintf(stderr, "ERROR: Raising the stake not allowed after 2 bets\n");
                throw 2;
            }
            else
            {
                cur_stake = bet;
                cur_player = other(cur_player);
            }
        }
    }
}
void SimplePoker::unmakeAction(int action_id)
{

}

void SimplePoker::_endOfBiddingPhase()
{
    agreed_stake = cur_stake;
    bidding_phase ++;
    if (bidding_phase > FINAL_BIDDING_PHASE)
    {
        //todo end of game, choose the winner
    }
    else
    {
        cur_player = RANDOM_PLAYER_NR;
        bids_number = 0;
    }
}

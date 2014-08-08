#include <vector>
#include <cstdio>
#include "HoldemPokerAbstraction.h"
#include "BasketManager.h"

using namespace std;

const int HoldemPokerAbstraction::ACTION_FOLD = 0;
const int HoldemPokerAbstraction::ACTION_CALL = 1;
const int HoldemPokerAbstraction::ACTION_RAISE = 2;
const int HoldemPokerAbstraction::ACTION_ALL_IN = 3;
const int HoldemPokerAbstraction::PHASE_FIRST_BID = 4;
const int HoldemPokerAbstraction::PHASE_MIDDLE_BID = 5;
const int HoldemPokerAbstraction::PHASE_LAST_BID = 6;
const int HoldemPokerAbstraction::LOG_MAX_STAKE = 8;
const int HoldemPokerAbstraction::IS_BASE = 5;
const int HoldemPokerAbstraction::NO_WINNER = -1;


void HoldemPokerAbstraction::_init()
{
    // 0 player always starts
    start_player = 0;
    cur_player = RANDOM_PLAYER_NR;
    cur_stake = 1;
    agreed_stake = 0;
    bidding_phase = 0;
    bids_number = 1;
    random_phase = 0;
    winner = NO_WINNER;
    is_id[0] = 1;
    is_id[1] = 2;
    is_id[2] = 3;
    player_basket[0] = player_basket[1] = 0;
}

HoldemPokerAbstraction::HoldemPokerAbstraction(BasketManager* mng)
{
    _init();
    manager = mng;
}

HoldemPokerAbstraction::~HoldemPokerAbstraction()
{

}

BasketManager* HoldemPokerAbstraction::getBasketManager()
{
    return manager;
}

void HoldemPokerAbstraction::makeAction(int action_id)
{
    _backup();
    if (cur_player == RANDOM_PLAYER_NR)
    {
        pair<int, int> basket_pair = decode_basket_pair(action_id);
        player_basket[0] = basket_pair.first;
        player_basket[1] = basket_pair.second;
        logAction(0, player_basket[0]);
        logAction(1, player_basket[1]);
        logAction(2, player_basket[0]);
        logAction(2, player_basket[1]);
        random_phase ++;
        _startOfBiddingPhase();
    }
    else
    {
        logAction(0, action_id);
        logAction(1, action_id);
        logAction(2, action_id);
        bids_number ++;
        /* player looses */
        if (action_id == ACTION_FOLD)
        {
            _endGame(other(cur_player));
        }
        if (action_id == ACTION_CALL)
        {
            agreed_stake = cur_stake;
            /* Second player agrees */
            if (bids_number >= 2)
                _endOfBiddingPhase();
            else
                cur_player = other(cur_player);
        }
        if (action_id == ACTION_RAISE)
        {
            agreed_stake = cur_stake;
            cur_stake += 1;
            cur_player = other(cur_player);
        }
        if (action_id == ACTION_ALL_IN)
        {
            agreed_stake = cur_stake;
            cur_stake = LOG_MAX_STAKE;
            cur_player = other(cur_player);
        }
    }
}

void HoldemPokerAbstraction::_startOfBiddingPhase()
{
    if (agreed_stake == LOG_MAX_STAKE)
        _endOfBiddingPhase();
    else
        cur_player = (start_player + bids_number) & 1;
}

vector<int> HoldemPokerAbstraction::getActionIds()
{
    return getActionIds(bids_number);
}

vector<int> HoldemPokerAbstraction::getActionIds(int bids_num)
{
    vector<int> action_ids;
    if (cur_stake > agreed_stake)
        action_ids.push_back(ACTION_FOLD);
    action_ids.push_back(ACTION_CALL);
    if (bids_num < MAX_BIDS_NUMBER && cur_stake < LOG_MAX_STAKE)
    {
        action_ids.push_back(ACTION_RAISE);
        action_ids.push_back(ACTION_ALL_IN);
    }
    return action_ids;
}

long long HoldemPokerAbstraction::getInformationSetId()
{
    return is_id[cur_player];
}

utility HoldemPokerAbstraction::getUtility()
{
    int stake = 0;
    if (agreed_stake > 0)
        stake = 1 << (agreed_stake - 1);
    if (winner == 0)
        return  make_pair(stake, -stake);
    else if (winner == 1)
        return make_pair(-stake, stake);
    else
        return make_pair(0.0, 0.0);
}

bool HoldemPokerAbstraction::isFinal()
{
    return winner != NO_WINNER;

}

void HoldemPokerAbstraction::_endGame(int win)
{
    winner = win;
}


dist HoldemPokerAbstraction::getActionDistribution()
{
    return manager -> getBasketPairsDistribution(random_phase, player_basket[0], player_basket[1]);
}

void HoldemPokerAbstraction::logAction(int pnum, int action_id)
{
    is_id[pnum] *= IS_BASE;
    is_id[pnum] += action_id;
}

void HoldemPokerAbstraction::_backup()
{
    AbsBackup *backup = new AbsBackup();
    backup -> cur_player = cur_player;
    backup -> agreed_stake = agreed_stake;
    backup -> cur_stake = cur_stake;
    backup -> random_phase = random_phase;
    backup -> bidding_phase = bidding_phase;
    backup -> bids_number = bids_number;
    backup -> winner = winner;
    backup -> prev = prev_backup;
    backup -> player_basket[0] = player_basket[0];
    backup -> player_basket[1] = player_basket[1];
    backup -> is_id[0] = is_id[0];
    backup -> is_id[1] = is_id[1];
    backup -> is_id[2] = is_id[2];
    prev_backup = backup;

}
void HoldemPokerAbstraction::_restore()
{
    cur_player = prev_backup -> cur_player;
    agreed_stake = prev_backup -> agreed_stake;
    cur_stake = prev_backup -> cur_stake;
    random_phase = prev_backup -> random_phase;
    bidding_phase = prev_backup -> bidding_phase;
    bids_number = prev_backup -> bids_number;
    winner = prev_backup -> winner;
    player_basket[0] = prev_backup -> player_basket[0];
    player_basket[1] = prev_backup -> player_basket[1];
    is_id[0] = prev_backup -> is_id[0];
    is_id[1] = prev_backup -> is_id[1];
    is_id[2] = prev_backup -> is_id[2];
    AbsBackup* temp = prev_backup -> prev;
    delete prev_backup;
    prev_backup = temp;

}

int HoldemPokerAbstraction::_evaluateHand(int pnum)
{
    // TODO check if we need to use expected value here
    return player_basket[pnum];
}

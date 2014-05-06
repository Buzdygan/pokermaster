#include <vector>
#include "HoldemPokerAbstraction.h"
#include "BasketManager.h"

using namespace std;

const int HoldemPokerAbstraction::ACTION_FOLD = 0;
const int HoldemPokerAbstraction::ACTION_CALL = 1;
const int HoldemPokerAbstraction::ACTION_RAISE = 2;
const int HoldemPokerAbstraction::PHASE_FIRST_BID = 4;
const int HoldemPokerAbstraction::PHASE_MIDDLE_BID = 5;
const int HoldemPokerAbstraction::PHASE_LAST_BID = 6;

int phase_actions[10];

void HoldemPokerAbstraction::_init()
{
    // 0 player always starts
    start_player = 0;
    cur_player = RANDOM_PLAYER_NR;
    cur_stake = agreed_stake = 1;
    bidding_phase = 0;
    bids_number = 1;
    random_phase = 0;
    is_final = false;
    results = make_pair(0.0, 0.0);
    is_id[0] = 0;
    is_id[1] = 1;
    mults[0] = mults[1] = 2;
}

HoldemPokerAbstraction::HoldemPokerAbstraction(BasketManager* mng)
{
    _init();
    manager = mng;
    for (int phase = 0; phase < 4; phase++)
        phase_actions[phase] = mng -> getBasketsNumber(phase);
    phase_actions[PHASE_FIRST_BID] = 2;
    phase_actions[PHASE_MIDDLE_BID] = 3;
    phase_actions[PHASE_LAST_BID] = 2;
}

HoldemPokerAbstraction::~HoldemPokerAbstraction()
{

}

void HoldemPokerAbstraction::makeAction(int action_id)
{
    _backup();
    if (cur_player == RANDOM_PLAYER_NR)
    {
        player_basket[0] = action_id % MAX_BASKETS_NUMBER;
        player_basket[1] = action_id / MAX_BASKETS_NUMBER;
        logAction(0, player_basket[0], random_phase);
        logAction(1, player_basket[1], random_phase);
        random_phase ++;
        _startOfBiddingPhase();
    }
    else
    {
        logAction(0, action_id, 4 + bids_number);
        logAction(1, action_id, 4 + bids_number);
        bids_number ++;
        /* player looses */
        if (action_id == ACTION_FOLD)
        {
            _endGame(other(cur_player));
        }
        if (action_id == ACTION_CALL)
        {
            /* Second player agrees */
            if (bids_number >= 2)
                _endOfBiddingPhase();
            else
                cur_player = other(cur_player);
        }
        if (action_id == ACTION_RAISE)
        {
            if (bids_number > MAX_BIDS_NUMBER)
            {
                fprintf(stderr, "ERROR: Raising the stake not allowed after 2 bets, there were %d bets\n", bids_number);
                throw 2;
            }
            else
            {
                agreed_stake = cur_stake;
                cur_stake *= 2;
                cur_player = other(cur_player);
            }
        }
    }
}

vector<int> HoldemPokerAbstraction::getActionIds()
{
    return getActionIds(bids_number);
}

vector<int> HoldemPokerAbstraction::getActionIds(int bids_num)
{
    vector<int> action_ids;
    if (bids_num > 0)
        action_ids.push_back(ACTION_FOLD);
    action_ids.push_back(ACTION_CALL);
    if (bids_num < MAX_BIDS_NUMBER)
        action_ids.push_back(ACTION_RAISE);
    return action_ids;
}

int HoldemPokerAbstraction::getInformationSetId()
{
    return is_id[cur_player];
}

dist HoldemPokerAbstraction::getActionDistribution()
{
    return manager -> getBasketPairsDistribution(random_phase, player_basket[0], player_basket[1]);
}

void HoldemPokerAbstraction::logAction(int pnum, int action_id, int phase_id)
{
    is_id[pnum] += (action_id + 1) * mults[pnum];
    mults[pnum] *= (phase_actions[phase_id] + 1);
}

void HoldemPokerAbstraction::logCards(int pnum, vector<int> cards, int random_phase)
{
    int cards_code = manager -> cardsCode(cards);
    int bnum = manager -> getNextBasket(random_phase,
                                                 player_basket[pnum],
                                                 cards_code);
    logAction(pnum, bnum, random_phase);
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
    backup -> results = results;
    backup -> is_final = is_final;
    backup -> prev = prev_backup;
    backup -> player_basket[0] = player_basket[0];
    backup -> player_basket[1] = player_basket[1];
    backup -> is_id[0] = is_id[0];
    backup -> is_id[1] = is_id[1];
    backup -> mults[0] = mults[0];
    backup -> mults[1] = mults[1];
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
    results = prev_backup -> results;
    is_final = prev_backup -> is_final;
    player_basket[0] = prev_backup -> player_basket[0];
    player_basket[1] = prev_backup -> player_basket[1];
    is_id[0] = prev_backup -> is_id[0];
    is_id[1] = prev_backup -> is_id[1];
    mults[0] = prev_backup -> mults[0];
    mults[1] = prev_backup -> mults[1];
    AbsBackup* temp = prev_backup -> prev;
    delete prev_backup;
    prev_backup = temp;

}

int HoldemPokerAbstraction::_evaluateHand(int pnum)
{
    // TODO check if we need to use expected value here
    return player_basket[pnum];
}

#include <vector>
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
    is_final = false;
    results = make_pair(0.0, 0.0);
    is_id[0] = 0;
    is_id[1] = 1;
    player_basket[0] = player_basket[1] = 0;
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

int HoldemPokerAbstraction::getInformationSetId()
{
    if (cur_player == RANDOM_PLAYER_NR)
        return -1;
    return is_id[cur_player];
}

void HoldemPokerAbstraction::_endGame(int winner)
{
    int stake = 0;
    if (agreed_stake > 0)
        stake = 1 << (agreed_stake - 1);
    if (winner == 0)
        results = make_pair(stake, -stake);
    else if (winner == 1)
        results = make_pair(-stake, stake);
    else
        results = make_pair(0.0, 0.0);
    is_final = true;
}


dist HoldemPokerAbstraction::getActionDistribution()
{
    return manager -> getBasketPairsDistribution(random_phase, player_basket[0], player_basket[1]);
}

void HoldemPokerAbstraction::logAction(int pnum, int action_id, int phase_id)
{
    //printf("is_id before: %d | ", is_id[pnum]);
    is_id[pnum] += (action_id + 1) * mults[pnum];
    //printf("logging action %d by %d in phase %d, is_id: %d, mults: %d\n", action_id, pnum,
    //                                                    phase_id, is_id[pnum], mults[pnum]);
    mults[pnum] *= (phase_actions[phase_id] + 1);
}

void HoldemPokerAbstraction::logCards(int pnum, vector<int> cards, int random_phase)
{
    int cards_code = manager -> cardsCode(cards);
    int bnum = manager -> getNextBasket(random_phase,
                                        player_basket[pnum],
                                        cards_code);
    log(1, "HPA: logCards: bnum: %d, random_phase: %d, player_basket[%d]: %d\n", bnum,
                                                                              random_phase,
                                                                              pnum,
                                                                              player_basket[pnum]);
    player_basket[pnum] = bnum;
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

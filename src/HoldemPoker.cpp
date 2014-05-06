#include <ctime>
#include <cstdlib>
#include <stack>
#include <algorithm>
#include "HoldemPoker.h"
#include "Utils.h"

const int HoldemPoker::DEFAULT_INITIAL_CASH = 100;
const int HoldemPoker::SMALL_BLIND = 4;
const int HoldemPoker::BIG_BLIND = 8;
const int HoldemPoker::CARDS_NUMBER = 52;
const int HoldemPoker::LAST_PHASE_NUMBER = 1;
const int HoldemPoker::DRAW = -1;
const int HoldemPoker::GAME_DRAWN = 6;
const int HoldemPoker::MAX_GAME_CARDS = 3;
const int HoldemPoker::MAX_STAKE = 5;
const int HoldemPoker::BLIND_RANDOM_PHASES = 4;
const int HoldemPoker::FINAL_BIDDING_PHASE = 3;
const int HoldemPoker::MAX_BIDS_NUMBER = 2;
const int HoldemPoker::GAME_IN_PROGRESS = 5;
const int HoldemPoker::ACTIONS_NUMBER = 3;
const int HoldemPoker::RANDOM_PHASE_CARDS_NUMBER[4] = {4,3,1,1};

void HoldemPoker::_init()
{
    // 0 player always starts
    start_player = 0;
    cur_player = RANDOM_PLAYER_NR;
    cur_stake = agreed_stake = 1;
    cards_dealt = 0;
    random_phase = 0;
    bidding_phase = 0;
    bids_number = 1;
    is_final = false;
    results = make_pair(0.0, 0.0);
    for (int c = 1; c <= CARDS_NUMBER; c++)
        deck.push_back(c);
}

HoldemPoker::HoldemPoker(HandEvaluator* eval)
{
    _init();
    evaluator = eval;
}

HoldemPoker::HoldemPoker()
{
    _init();
}

HoldemPoker::~HoldemPoker()
{
}

int HoldemPoker::getInformationSetId()
{
    return -1;
}

utility HoldemPoker::getUtility()
{
    return results;
}

int HoldemPoker::getPlayerId()
{
    return cur_player;
}

dist HoldemPoker::getActionDistribution()
{
    dist d;
    double prob = 1.0 / deck.size();
    for (int i = 0; i < (int) deck.size(); i++)
        d.push_back(make_pair(deck[i], prob));
    return d;
}

/* returns player that sees the random action.*/
int HoldemPoker::randomActionPlayer()
{
    if (random_phase == 0)
        return cards_dealt & 1;
    return ALL_PLAYERS;
}

bool HoldemPoker::isFinal()
{
    return is_final;
}

vector<int> HoldemPoker::getActionIds()
{
    vector<int> action_ids;
    action_ids.push_back(0);
    if (bids_number == MAX_BIDS_NUMBER)
    {
        action_ids.push_back(cur_stake);
    }
    else
    {
        for (int i = cur_stake; i <= MAX_STAKE; i++)
            action_ids.push_back(i);
    }
    return action_ids;
}

void HoldemPoker::_endGame(int winner)
{
    if (winner == 0)
        results = make_pair(agreed_stake, -agreed_stake);
    else if (winner == 1)
        results = make_pair(-agreed_stake, agreed_stake);
    else
        results = make_pair(0.0, 0.0);
    is_final = true;
}

void HoldemPoker::makeAction(int action_id)
{
    _backup();
    /* if it's random player turn, action_id = dealt card id
     * otherwise it's the bet of a player. First player bids 1. */
    /* random player, action_id = card id */
    if (cur_player == RANDOM_PLAYER_NR)
    {
        int seeing_player = randomActionPlayer();
        int card_id = action_id;
        /* remove the card from the deck */
        deck.erase(std::remove(deck.begin(), deck.end(), card_id), deck.end());

        /* card for both players */
        if (seeing_player == ALL_PLAYERS)
        {
            for (int p = 0; p < 2; p++)
                player_cards[p].push_back(card_id);
        }
        /* card for single player */
        else
            player_cards[seeing_player].push_back(card_id);

        cards_dealt ++;
        /* We've dealt all the cards for this phase */
        if (cards_dealt == RANDOM_PHASE_CARDS_NUMBER[random_phase])
        {
            random_phase ++;
            cards_dealt = 0;
            _startOfBiddingPhase();
        }
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
            _endGame(other(cur_player));
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
            if (bids_number > MAX_BIDS_NUMBER)
            {
                fprintf(stderr, "ERROR: Raising the stake not allowed after 2 bets, there were %d bets\n", bids_number);
                throw 2;
            }
            else
            {
                agreed_stake = cur_stake;
                cur_stake = bet;
                cur_player = other(cur_player);
            }
        }
    }
}
void HoldemPoker::unmakeAction(int action_id)
{
    _restore();
}


void HoldemPoker::_startOfBiddingPhase()
{
    cur_player = (start_player + bids_number) & 1;
}

void HoldemPoker::_endOfBiddingPhase()
{
    bids_number = 0;
    bidding_phase ++;
    if (bidding_phase > FINAL_BIDDING_PHASE)
    {
        int strength0 = _evaluateHand(0);
        int strength1 = _evaluateHand(1);
        if (strength0 > strength1)
            _endGame(0);
        else if (strength1 > strength0)
            _endGame(1);
        else
            _endGame(DRAW);
    }
    else
        cur_player = RANDOM_PLAYER_NR;
}

int HoldemPoker::_evaluateHand(int pnum)
{
    return evaluator -> evaluateHand(player_cards[pnum]);
}

void HoldemPoker::_backup()
{
    Backup *backup = new Backup();
    backup -> cur_player = cur_player;
    backup -> agreed_stake = agreed_stake;
    backup -> cur_stake = cur_stake;
    backup -> cards_dealt = cards_dealt;
    backup -> random_phase = random_phase;
    backup -> bidding_phase = bidding_phase;
    backup -> bids_number = bids_number;
    backup -> results = results;
    backup -> is_final = is_final;
    backup -> deck = deck;
    backup -> player_cards[0] = player_cards[0];
    backup -> player_cards[1] = player_cards[1];
    backup -> prev = prev_backup;
    prev_backup = backup;
}

void HoldemPoker::_restore()
{
    cur_player = prev_backup -> cur_player;
    agreed_stake = prev_backup -> agreed_stake;
    cur_stake = prev_backup -> cur_stake;
    cards_dealt = prev_backup -> cards_dealt;
    random_phase = prev_backup -> random_phase;
    bidding_phase = prev_backup -> bidding_phase;
    bids_number = prev_backup -> bids_number;
    results = prev_backup -> results;
    is_final = prev_backup -> is_final;
    deck = prev_backup -> deck;
    player_cards[0] = prev_backup -> player_cards[0];
    player_cards[1] = prev_backup -> player_cards[1];
    Backup* temp = prev_backup -> prev;
    delete prev_backup;
    prev_backup = temp;
}

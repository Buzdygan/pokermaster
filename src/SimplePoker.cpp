#include <ctime>
#include <cstdlib>
#include <stack>
#include <algorithm>
#include <fstream>
#include "SimplePoker.h"
#include "Utils.h"

const int SimplePoker::DEFAULT_INITIAL_CASH = 100;
const int SimplePoker::SMALL_BLIND = 4;
const int SimplePoker::BIG_BLIND = 8;
const int SimplePoker::CARDS_NUMBER = 6;
const int SimplePoker::LAST_PHASE_NUMBER = 1;
const int SimplePoker::DRAW = -1;
const int SimplePoker::GAME_DRAWN = 6;
const int SimplePoker::MAX_GAME_CARDS = 3;
const int SimplePoker::MAX_STAKE = 5;
const int SimplePoker::FINAL_BIDDING_PHASE = 1;
const int SimplePoker::FINAL_RANDOM_PHASE = 2;
const int SimplePoker::MAX_BIDS_NUMBER = 2;
const int SimplePoker::GAME_IN_PROGRESS = 5;
const int SimplePoker::PHASE_TABLE_CARDS[2][2][2] = {{{0,1}, {2,3}}, {{1,2}, {2,3}}};

SimplePoker::SimplePoker()
{
    // 0 player always starts
    start_player = 0;
    information_set_ids[0] = 1;
    information_set_ids[1] = 2;
    cur_player = RANDOM_PLAYER_NR;
    cur_stake = agreed_stake = 1;
    bidding_phase = 0;
    bids_number = 0;
    random_phase = 0;
    is_final = false;
    results = make_pair(0.0, 0.0);
    for (int c = 1; c <= CARDS_NUMBER; c++)
        deck.push_back(c);
}

SimplePoker::~SimplePoker()
{
}

long long SimplePoker::getInformationSetId()
{
    if (cur_player == RANDOM_PLAYER_NR)
        return -1;
    return information_set_ids[cur_player];
}

utility SimplePoker::getUtility()
{
    return results;
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
    return is_final;
}

int SimplePoker::bidsNumber()
{
    return bids_number;
}

vector<int> SimplePoker::getActionIds()
{
    if (cur_player == RANDOM_PLAYER_NR)
        return deck;
    else
    {
        int max_bet = MAX_STAKE;
        if (bids_number >= MAX_BIDS_NUMBER)
            max_bet = cur_stake;
        vector<int> action_ids;
        action_ids.push_back(0);
        for (int i = cur_stake; i <= max_bet; i++)
            action_ids.push_back(i);
        return action_ids;
    }
}

void SimplePoker::_endGame(int winner)
{
    if (winner == 0)
        results = make_pair(agreed_stake, -agreed_stake);
    else if (winner == 1)
        results = make_pair(-agreed_stake, agreed_stake);
    else
        results = make_pair(0.0, 0.0);
    is_final = true;
}

void SimplePoker::makeAction(int action_id)
{
    _backup();
    /* if it's random player turn, action_id = dealt card id
     * otherwise it's the bet of a player. First player has to bid at least 1. */
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
            {
                player_cards[0].push_back(card_id);
                player_cards[1].push_back(card_id);
                _logAction(card_id - 1, p);
            }
            _startOfBiddingPhase();
        }
        /* card for single player */
        else
        {
            player_cards[seeing_player].push_back(card_id);
            _logAction(card_id - 1, seeing_player);
        }
        random_phase ++;
        if (random_phase == FINAL_RANDOM_PHASE)
            _startOfBiddingPhase();
    }
    else
    {
        bids_number ++;
        int bet = action_id;
        _logAction(bet, 0);
        _logAction(bet, 1);
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
    _restore();
}

void SimplePoker::_startOfBiddingPhase()
{
    cur_player = start_player;
    bids_number = 0;
}

void SimplePoker::_endOfBiddingPhase()
{
    agreed_stake = cur_stake;
    bidding_phase ++;
    if (bidding_phase > FINAL_BIDDING_PHASE)
    {
        int strength0 = _evaluateHand(player_cards[0]);
        int strength1 = _evaluateHand(player_cards[1]);
        //printf("strength0: %d, strength1: %d\n", strength0, strength1);
        if (strength0 > strength1)
            _endGame(0);
        else if (strength1 > strength0)
            _endGame(1);
        else
            _endGame(DRAW);
    }
    else
    {
        cur_player = RANDOM_PLAYER_NR;
        bids_number = 0;
    }
}

int SimplePoker::_evaluateHand(vector<int> cards)
{
    int c0 = (cards[0] + 1) / 2, c1 = (cards[1] + 1) / 2;
    if (c0 == c1)
        return 4 + c0;
    return c0 + c1 - 2;

}

void SimplePoker::_backup()
{
    Backup *backup = new Backup();
    backup -> cur_player = cur_player;
    backup -> agreed_stake = agreed_stake;
    backup -> cur_stake = cur_stake;
    backup -> random_phase = random_phase;
    backup -> bidding_phase = bidding_phase;
    backup -> bids_number = bids_number;
    backup -> results = results;
    backup -> is_final = is_final;
    backup -> deck = deck;
    backup -> player_cards[0] = player_cards[0];
    backup -> player_cards[1] = player_cards[1];
    backup -> information_set_ids[0] = information_set_ids[0];
    backup -> information_set_ids[1] = information_set_ids[1];
    backup -> prev = prev_backup;
    prev_backup = backup;
}

void SimplePoker::_restore()
{
    cur_player = prev_backup -> cur_player;
    agreed_stake = prev_backup -> agreed_stake;
    cur_stake = prev_backup -> cur_stake;
    random_phase = prev_backup -> random_phase;
    bidding_phase = prev_backup -> bidding_phase;
    bids_number = prev_backup -> bids_number;
    results = prev_backup -> results;
    is_final = prev_backup -> is_final;
    deck = prev_backup -> deck;
    player_cards[0] = prev_backup -> player_cards[0];
    player_cards[1] = prev_backup -> player_cards[1];
    information_set_ids[0] = prev_backup -> information_set_ids[0];
    information_set_ids[1] = prev_backup -> information_set_ids[1];
    Backup* temp = prev_backup -> prev;
    delete prev_backup;
    prev_backup = temp;
}

void SimplePoker::_logAction(int action_id, int seeing_player)
{
    int base = CARDS_NUMBER;
    if (MAX_STAKE + 1 > base)
        base = MAX_STAKE + 1;
    information_set_ids[seeing_player] *= base;
    information_set_ids[seeing_player] += action_id;
}

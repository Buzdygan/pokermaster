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

HoldemPoker::HoldemPoker()
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

HoldemPoker::~HoldemPoker()
{
}

int HoldemPoker::getInformationSetId()
{
    if (cur_player == RANDOM_PLAYER_NR)
        return -1;
    return information_set_ids[cur_player];
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
    if (random_phase < BLIND_RANDOM_PHASES)
        return random_phase & 1;
    return ALL_PLAYERS;
}

bool HoldemPoker::isFinal()
{
    return is_final;
}

vector<int> HoldemPoker::getActionIds()
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
                player_cards[p].push_back(card_id);
        }
        /* card for single player */
        else
            player_cards[seeing_player].push_back(card_id);

        random_phase ++;
        if (random_phase == 4 || random_phase >= 7)
        {
            if (random_phase == 4)
            {
                _logCards(0, 0, 1);
                _logCards(1, 0, 1);
            }
            else if (random_phase == 7)
            {
                _logCards(0, 2, 4);
                _logCards(1, 2, 4);
            }
            else
            {
                _logCards(0, random_phase - 3, random_phase - 3);
                _logCards(1, random_phase - 3, random_phase - 3);
            }
            _startOfBiddingPhase();
        }
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
void HoldemPoker::unmakeAction(int action_id)
{
    _restore();
}

void HoldemPoker::_startOfBiddingPhase()
{
    cur_player = start_player;
    bids_number = 0;
}

void HoldemPoker::_endOfBiddingPhase()
{
    agreed_stake = cur_stake;
    bidding_phase ++;
    if (bidding_phase > FINAL_BIDDING_PHASE)
    {
        int strength0 = _evaluateHand(player_cards[0]);
        int strength1 = _evaluateHand(player_cards[1]);
        printf("strength0: %d, strength1: %d\n", strength0, strength1);
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

int HoldemPoker::_evaluateHand(vector<int> cards)
{
    return evaluator.evaluateHand(cards);
}

void HoldemPoker::_backup()
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

void HoldemPoker::_restore()
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

void HoldemPoker::_logAction(int action_id, int seeing_player)
{
    int base = CARDS_NUMBER;
    if (MAX_STAKE + 1 > base)
        base = MAX_STAKE + 1;
    information_set_ids[seeing_player] *= base;
    information_set_ids[seeing_player] += action_id;
}

void HoldemPoker::_logCards(int seeing_player, int ind0, int ind1)
{

}

HandEvaluator::HandEvaluator()
{
	memset(HR, 0, sizeof(HR));
	FILE * fin = fopen("../data/HandRanks.dat", "rb");
	size_t bytesread = fread(HR, sizeof(HR), 1, fin);	// get the HandRank Array
	fclose(fin);
}

int HandEvaluator::evaluateHand(vector<int> cards)
{
    int ind = 0;
    int p = HR[53 + cards[ind++]];
    p = HR[p + cards[ind++]];
    p = HR[p + cards[ind++]];
    p = HR[p + cards[ind++]];
    p = HR[p + cards[ind++]];
    p = HR[p + cards[ind++]];
    return HR[p + cards[ind++]];
}

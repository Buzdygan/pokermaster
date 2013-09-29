#include <cstdlib>
#include <ctime>
#include <cstring>
#include <algorithm>

#include "Player.h"
#include "Utils.h"
#include "Poker.h"

using namespace std;

Poker::Poker(Player* player0, Player* player1)
{
    players[0] = player0; 
    players[1] = player1; 
    for (int i = 1; i <= CARDS_NUMBER; i++)
        deck.push_back(i);
}

void Poker::play(int p0_cash, int p1_cash)
{
    pcash[0] = p0_cash;
    pcash[1] = p1_cash;
    round_number = 0;
    nextRound(DRAW); // starting round

    while (gameWinner() == GAME_IN_PROGRESS) // playing round 
    {
        players[0] -> startNewRound(playerCards(0));
        players[1] -> startNewRound(playerCards(1));
        int this_round_number = round_number;
        while(round_number == this_round_number) // playing phase 
        {
            int bet = -1;
            int this_phase_number = phase;
            for (int i = 0; i < PHASE_TABLE_CARDS[phase]; i ++) // dealing table cards
            {
                int card = getTableCard();
                players[0] -> showTableCard(card);
                players[1] -> showTableCard(card);
            }
            while (phase == this_phase_number && round_number == this_round_number)
            {
                bet = players[cur_player] -> getBet(bet);
                makeBet(bet);
            }
        }
        printf("cash0: %d, cash1: %d\n", pcash[0], pcash[1]);
    }
    printf("Winner of the game is player number %d\n", gameWinner());
}

/* bet_raise signifies amount by which current player want to raise the stake */
void Poker::makeBet(int bet_raise)
{
    int oth_player = other(cur_player);
    int cur_bet = bets[oth_player];
    phase_move ++;
    if (stake + bet_raise > pcash[cur_player])
    {
        fprintf(stderr, "Raising more than the player has");
        throw;
    }
    if (bet_raise < cur_bet) // fold or all in
    {
        if (stake + bet_raise == pcash[cur_player]) // all in
        {
            stake += bet_raise;
            finishRound();
        }
        else // fold
            nextRound(oth_player);
        return;
    }
    if (phase_move > 1 && bet_raise == cur_bet) // call
    {
        stake += cur_bet;
        if (stake == pcash[0] || stake == pcash[1]) // someone went all in
            finishRound();
        else
            nextPhase();
    }
    else if (bet_raise > cur_bet) // raise
    {
        if (phase_move >= 3)
        {
            fprintf(stderr, "Raising in third move\n");
            throw;
        }
        bets[cur_player] = bet_raise;
    }
    cur_player = oth_player;
}

int Poker::getTableCard()
{
    if (table_card_index >= MAX_GAME_CARDS) {
        fprintf(stderr, "Drawing too many cards\n");
        throw;
    }
    phase ++;
    return deck[table_card_index ++];
}

pair<int, int> Poker::playerCards(int player_number)
{
    int first = deck[player_number * 2];
    int second = deck[player_number * 2 + 1];
    return make_pair(first, second);
}

int Poker::gameWinner()
{
    if (phase == 0)
    {
        if (pcash[0] == 0)
            return 1;
        if (pcash[1] == 0)
            return 0;
    }
    return GAME_IN_PROGRESS;
}

void Poker::finishRound()
{
    // TODO show other players' cards
    while (table_card_index < MAX_GAME_CARDS)
    {
        int card = deck[table_card_index ++];
        players[0] -> showTableCard(card);
        players[1] -> showTableCard(card);
    }
    nextRound(checkRoundWinner());
}

void Poker::nextRound(int winner)
{
    if (winner != DRAW)
    {
        pcash[winner] += stake;
        pcash[other(winner)] -= stake;
    }
    players[0] -> announceRoundWinner(winner, stake);
    players[1] -> announceRoundWinner(winner, stake);
    round_number ++;
    phase = phase_move = stake = 0;
    bets[0] = bets[1] = 0;
    random_shuffle(deck.begin(), deck.end());
    table_card_index = 4;
    cur_player = round_number & 1;
    bets[cur_player] = SMALL_BLIND;
    bets[other(cur_player)] = BIG_BLIND;
}

void Poker::nextPhase()
{
    if (phase == LAST_PHASE_NUMBER) // end of Round
        nextRound(checkRoundWinner());
    else
    {
        phase ++;
        bets[0] = bets[1] = phase_move = 0;
        cur_player = round_number & 1;
    }
}

int Poker::checkRoundWinner()
{
    vector<int> pCards[2];
    for (int p = 0; p < 2; p++)
    {
        for (int i = 4; i < 9; i++)
            pCards[p].push_back(deck[i]);
        pCards[p].push_back(deck[2 * p]);
        pCards[p].push_back(deck[2 * p + 1]);
    }
    int score0 = evaluateCards(pCards[0]); 
    int score1 = evaluateCards(pCards[1]); 
    if (score0 == score1)
        return DRAW;
    return score0 > score1 ? 0 : 1;
}

int Poker::other(int player_number)
{
    return (player_number + 1) & 1;
}

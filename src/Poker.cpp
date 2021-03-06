#include <cstdlib>
#include <ctime>
#include <cstring>
#include <algorithm>

#include "Player.h"
#include "Utils.h"
#include "Poker.h"

using namespace std;

Poker::Poker(Player* player0, Player* player1, int players_cash)
{
    pcash = players_cash;
    players[0] = player0;
    players[1] = player1;
    for (int i = 1; i <= CARDS_NUMBER; i++)
        deck.push_back(i);
    evaluator = new HandEvaluator();
}

void Poker::play(int total_rounds_number)
{
    round_number = 0;
    cash_won[0] = cash_won[1] = 0;
    nextRound(DRAW); // starting round
    while (round_number <= total_rounds_number)
    {
        players[0] -> startNewRound(playerCards(0));
        players[1] -> startNewRound(playerCards(1));
        int this_round_number = round_number;
        nextPhase();
        while(round_number == this_round_number) // playing phases
        {
            printf("DEBUG: phase %d\n", phase);
            int bet = (phase == 0) ? BIG_BLIND : 0;
            int this_phase_number = phase;
            while (phase == this_phase_number && round_number == this_round_number)
            {
                bet = players[cur_player] -> getBet(bet);
                makeBet(bet);
            }
        }
        printf("cash0: %d, cash1: %d\n", cash_won[0], cash_won[1]);
    }
    int winner = gameWinner();
    if (winner == GAME_DRAWN)
        printf("Game has been drawn\n");
    else
        printf("Winner of the game is player number %d\n", winner);
}

/* bet_raise signifies amount by which current player want to raise the stake */
void Poker::makeBet(int bet_raise)
{
    int oth_player = other(cur_player);
    int cur_bet = bets[oth_player];
    printf("current player: %d, bet_raise: %d\n", cur_player, bet_raise);
    phase_move ++;
    if (stake + bet_raise > pcash)
    {
        fprintf(stderr, "Raising more than the player has");
        throw;
    }
    if (bet_raise < cur_bet) // fold or all in
    {
        if (stake + bet_raise == pcash) // all in
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
        if (stake == pcash) // someone went all in
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
    printf("cash_won 0: %d\n", cash_won[0]);
    printf("cash_won 1: %d\n", cash_won[1]);
    if (cash_won[0] == cash_won[1])
        return GAME_DRAWN;
    return (cash_won[0] > cash_won[1]) ? 0 : 1;
}

void Poker::finishRound()
{
    // TODO show other players' cards
    nextRound(checkRoundWinner());
}

void Poker::nextRound(int winner)
{
    printf("DEBUG: nextRound\n");
    if (winner != DRAW)
    {
        printf("cash won %d;\n", stake);
        cash_won[winner] += stake;
        printf("cash_won 0: %d\n", cash_won[0]);
        printf("cash_won 1: %d\n", cash_won[1]);
    }
    printf("DEBUG: winner of the round is player %d\n", winner);
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
    printf("DEBUG: nextPhase, phase %d\n", phase);
    if (phase == LAST_PHASE_NUMBER) // end of Round
        nextRound(checkRoundWinner());
    else
    {
        for (int p = 0; p < 2; p++)
        {
            vector<int> cards;
            for (int c = PHASE_TABLE_CARDS[p][phase][0]; c < PHASE_TABLE_CARDS[p][phase][1]; c++)
                cards.push_back(deck[c]);
            players[p] -> startNewPhase(cards);
        }
        phase ++;
        bets[0] = bets[1] = phase_move = 0;
        cur_player = round_number & 1;
    }
}

int Poker::checkRoundWinner()
{
    vector<int> pCards[2];
    for (int p = 0; p < 2; p++)
        for (int phase = 0; phase <= LAST_PHASE_NUMBER; phase++)
            for (int ind = PHASE_TABLE_CARDS[p][phase][0]; ind < PHASE_TABLE_CARDS[p][phase][1]; ind++)
                pCards[p].push_back(deck[ind]);

    int score0 = evaluator->evaluateHand(pCards[0]);
    int score1 = evaluator->evaluateHand(pCards[1]);
    printf("DEBUG: checkRoundWInner, score0: %d, score1: %d\n", score0, score1);
    if (score0 == score1)
        return DRAW;
    return score0 > score1 ? 0 : 1;
}


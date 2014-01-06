#ifndef POKER_H
#define POKER_H

/* betting rules
 * In each phase players can only raise in their first move. So there are maximum 3 moves.
 */
#include <utility>
#include <vector>

#include "Player.h"
#include "Utils.h"

using namespace std;


class Poker
{
    public:
        Poker(Player* player0, Player* player1, int pcash);
        void play(int rounds_number);

    protected:
        /* Round variables */
        int phase;
        int phase_move;
        int pcash;
        int stake;
        int cur_player;
        int table_card_index;
        int cash_won [2];
        int bets [2];
        bool table_draw_mode;

        /* Game variables */
        Player* players[2];
        HandEvaluator *evaluator;
        int round_number;
        vector<int> deck;

        /* Record current player's bet */
        void makeBet(int bet_raise);
        /* draws next table card */
        int getTableCard();
        /* returns pair of cards for the given player number */
        pair<int, int> playerCards(int player_number);
        int gameWinner();
        /* Determines who is the winner of the current round */
        int checkRoundWinner();
        /* Redistributes the stake, starts new round, redeals cards etc */
        void nextRound(int winner);
        /* When someone played all in, finishes the round, dealing rest of cards and announcing winner */
        void finishRound();
        /* Progresses to the next phase */
        void nextPhase();
};


#endif

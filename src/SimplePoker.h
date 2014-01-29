#ifndef SIMPLE_POKER_H
#define SIMPLE_POKER_H

#include "GameAbstraction.h"

/* RULES
 * Each player is dealt one private card.
 * Then there is bidding phase.
 * Finally one shared card is dealt and another bidding phase before the end */


class SimplePoker : public GameAbstraction
{
    public:
        int getInformationSetId();
        utility getUtility();
        int getPlayerId();
        dist getActionDistribution();
        /* returns player that sees the random action.
         * -1 means all players */
        int randomActionPlayer();
        bool isFinal();
        vector<int> getActionIds();
        void makeAction(int action_id);
        void unmakeAction(int action_id);
        ~SimplePoker();
        SimplePoker();
    private:
        int start_player;
        int cur_player;
        int agreed_stake;
        int cur_stake;
        int random_phase;
        int bidding_phase;
        int bids_number;
        vector<int> deck;
        vector<int> player_cards[2];

        void _endOfBiddingPhase();
        void endGame(int winner);
};


#endif

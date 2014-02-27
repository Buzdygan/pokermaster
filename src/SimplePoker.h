#ifndef SIMPLE_POKER_H
#define SIMPLE_POKER_H

#include <stack>
#include "GameAbstraction.h"

/* RULES
 * Each player is dealt one private card.
 * Then there is bidding phase.
 * Finally one shared card is dealt and another bidding phase before the end */

struct Backup
{
    Backup *prev;
    int cur_player;
    int agreed_stake;
    int cur_stake;
    int random_phase;
    int bidding_phase;
    int bids_number;
    utility results;
    bool is_final;
    vector<int> deck;
    vector<int> player_cards[2];
    int information_set_ids[2];
};

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
        /* results of each player after the round is finished */
        utility results;
        /* has the game ended yet */
        bool is_final;
        vector<int> deck;
        vector<int> player_cards[2];
        int information_set_ids[2];
        Backup *prev_backup;

        void _endOfBiddingPhase();
        void _startOfBiddingPhase();
        void _backup();
        void _restore();
        void _endGame(int winner);
        void _logAction(int action_id, int seeing_player);
};



#endif

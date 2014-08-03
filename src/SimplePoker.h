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
        long long getInformationSetId();
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
        int bidsNumber();
        ~SimplePoker();
        SimplePoker();

        static const int DEFAULT_INITIAL_CASH;
        static const int SMALL_BLIND;
        static const int BIG_BLIND;
        static const int CARDS_NUMBER;
        static const int LAST_PHASE_NUMBER;
        static const int DRAW;
        static const int GAME_DRAWN;
        static const int MAX_GAME_CARDS;
        static const int MAX_STAKE;
        static const int BLIND_RANDOM_PHASES;
        static const int FINAL_RANDOM_PHASE;
        static const int FINAL_BIDDING_PHASE;
        static const int MAX_BIDS_NUMBER;
        static const int GAME_IN_PROGRESS;
        static const int PHASE_TABLE_CARDS[2][2][2];

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

        int _evaluateHand(vector<int> cards);
        void _endOfBiddingPhase();
        void _startOfBiddingPhase();
        void _backup();
        void _restore();
        void _endGame(int winner);
        void _logAction(int action_id, int seeing_player);
};



#endif

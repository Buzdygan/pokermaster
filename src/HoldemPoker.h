#ifndef __HOLDEM_POKER_H
#define __HOLDEM_POKER_H

#include <stack>
#include "GameAbstraction.h"
#include "Utils.h"

/* RULES
 * TODO
 */

struct Backup
{
    Backup *prev;
    int cur_player;
    int agreed_stake;
    int cur_stake;
    int cards_dealt;
    int random_phase;
    int bidding_phase;
    int bids_number;
    utility results;
    bool is_final;
    vector<int> deck;
    vector<int> player_cards[2];
};


class HoldemPoker : public GameAbstraction
{
    public:
        virtual long long getInformationSetId();
        virtual dist getActionDistribution();
        virtual vector<int> getActionIds();
        virtual void makeAction(int action_id);
        virtual utility getUtility();
        int getPlayerId();
        /* returns player that sees the random action.
         * -1 means all players */
        int randomActionPlayer();
        virtual bool isFinal();
        void unmakeAction(int action_id);
        ~HoldemPoker();
        HoldemPoker();
        HoldemPoker(HandEvaluator* evaluator);

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
        static const int FINAL_BIDDING_PHASE;
        static const int MAX_BIDS_NUMBER;
        static const int GAME_IN_PROGRESS;
        static const int ACTIONS_NUMBER;
        static const int RANDOM_PHASE_CARDS_NUMBER[4];
    protected:
        int start_player;
        int cur_player;
        int agreed_stake;
        int cur_stake;
        int cards_dealt;
        int random_phase;
        int bidding_phase;
        int bids_number;
        /* results of each player after the round is finished */
        utility results;
        /* has the game ended yet */
        bool is_final;

        virtual void _backup();
        virtual void _restore();
        virtual int _evaluateHand(int player_number);
        virtual void _init();
        void _endOfBiddingPhase();
        virtual void _startOfBiddingPhase();
        virtual void _endGame(int winner);

    private:
        vector<int> deck;
        vector<int> player_cards[2];
        Backup *prev_backup;
        HandEvaluator* evaluator;
};



#endif

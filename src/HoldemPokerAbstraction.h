#ifndef __HOLDEM_POKER_ABSTRACTION_H
#define __HOLDEM_POKER_ABSTRACTION_H

#include "HoldemPoker.h"
#include "BasketManager.h"

struct AbsBackup
{
    AbsBackup *prev;
    int cur_player;
    int agreed_stake;
    int cur_stake;
    int random_phase;
    int bidding_phase;
    int bids_number;
    int winner;
    int player_basket[2];
    long long is_id[2];
};

class HoldemPokerAbstraction : public HoldemPoker
{
    public:
        HoldemPokerAbstraction(BasketManager* manager);
        ~HoldemPokerAbstraction();

        long long getInformationSetId();
        dist getActionDistribution();
        virtual vector<int> getActionIds();
        virtual vector<int> getActionIds(int bids_num);
        void makeAction(int action_id);
        void logAction(int pnum, int action_id);
        utility getUtility();
        bool isFinal();
        BasketManager* getBasketManager();

        static const int ACTION_FOLD;
        static const int ACTION_CALL;
        static const int ACTION_RAISE;
        static const int ACTION_ALL_IN;
        static const int PHASE_FIRST_BID;
        static const int PHASE_MIDDLE_BID;
        static const int PHASE_LAST_BID;
        static const int LOG_MAX_STAKE;
        static const int IS_BASE;
        static const int NO_WINNER;

    protected:
        BasketManager *manager;
        int player_basket[2];
        long long is_id[2];
        int winner;
        AbsBackup* prev_backup;

        void _init();
        void _backup();
        void _restore();
        void _startOfBiddingPhase();
        void _endGame(int winner);
        int _evaluateHand(int player_num);
};






#endif


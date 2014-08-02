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
    utility results;
    bool is_final;
    int player_basket[2];
    int is_id[2];
    int mults[2];
};

class HoldemPokerAbstraction : public HoldemPoker
{
    public:
        HoldemPokerAbstraction(BasketManager* manager);
        ~HoldemPokerAbstraction();

        virtual int getInformationSetId();
        dist getActionDistribution();
        vector<int> getActionIds();
        vector<int> getActionIds(int bids_num);
        void makeAction(int action_id);
        void logAction(int pnum, int action_id, int phase_id);
        void logCards(int pnum, vector<int> cards, int random_phase);
        BasketManager* getBasketManager();

        static const int ACTION_FOLD;
        static const int ACTION_CALL;
        static const int ACTION_RAISE;
        static const int ACTION_ALL_IN;
        static const int PHASE_FIRST_BID;
        static const int PHASE_MIDDLE_BID;
        static const int PHASE_LAST_BID;
        static const int LOG_MAX_STAKE;

    protected:
        BasketManager *manager;
        int player_basket[2];
        int phase_actions[10];
        int is_id[2];
        int mults[2];
        AbsBackup* prev_backup;

        void _init();
        void _backup();
        void _restore();
        void _startOfBiddingPhase();
        void _endGame(int winner);
        int _evaluateHand(int player_num);
};






#endif


#ifndef SIMPLE_POKER_H
#define SIMPLE_POKER_H

#include "GameAbstraction.h"

const int MAX_STAKE = 5;
const int CARDS_NUMBER = 6;
const int DRAW = -1;
const int FINAL_RANDOM_PHASE = 2;

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
        vector<int> deck;
        vector<int> player_cards[2];

        void endGame(int winner);
};


#endif

#ifndef SIMPLE_POKER_H
#define SIMPLE_POKER_H

#include "GameAbstraction.h"



class SimplePoker : public GameAbstraction
{
    public:
        int getInformationSetId();
        utility getUtility();
        int getPlayerId();
        dist getActionDistribution();
        bool isFinal();
        vector<int> getActionIds();
        void makeAction(int action_id);
        void unmakeAction(int action_id);
        ~SimplePoker();
        SimplePoker();

};


#endif

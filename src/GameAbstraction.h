#ifndef GAME_ABSTRACTION_H
#define GAME_ABSTRACTION_H

#include <vector>
#include <map>
#include <cstdlib>
using namespace std;

const int RANDOM_PLAYER_NR = 2;
typedef pair<double, double> utility;
typedef vector<pair<int, double> > dist;

typedef vector<int>::iterator vi_it;
typedef map<pair<int,int>, double>::iterator stg_it;
typedef dist::iterator dist_it;

class GameAbstraction
{
    public:
        virtual int getInformationSetId() = 0;
        virtual utility getUtility() = 0;
        virtual int getPlayerId() = 0;
        virtual dist getActionDistribution() = 0;
        virtual bool isFinal() = 0;
        virtual vector<int> getActionIds() = 0;
        virtual void makeAction(int action_id) = 0;
        virtual void unmakeAction(int action_id) = 0;
        virtual ~GameAbstraction();
    protected:
        GameAbstraction() {}
};



#endif

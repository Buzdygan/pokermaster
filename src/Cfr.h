#ifndef CFR_H
#define CFR_H

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

class CfrGameAbstraction
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
    protected:
        CfrGameAbstraction() {}
};


class Cfr
{
    public:
        Cfr(CfrGameAbstraction* game);
        int getActionId();
        void makeAction(int action_id);

    private:
        static const char* DEFAULT_FILE;
        static const char FILE_DELIM;
        CfrGameAbstraction* game;
        bool initialized;

        map<pair<int, int>, double> strategy;
        map<pair<int, int>, double> R;
        map<pair<int, int>, double> S;

        bool loadFromFile(const char* filename);
        void saveToFile(const char* filename);

        void computeVanillaCfr();
        utility walkTree(double probs[3]);
};

#endif

#ifndef CFR_H
#define CFR_H

#include <vector>
#include <map>
#include <cstdlib>
#include "GameAbstraction.h"
using namespace std;

typedef map<pair<long long, int>, double> Smap;
typedef map<pair<long long, int>, double>::iterator Sit;

class Cfr
{
    public:
        Cfr(GameAbstraction* game, int iterations=ITERATIONS, const char* strategy_file=DEFAULT_FILE, bool information_tree=true);
        int getActionId(long long information_set_id, vector<int> action_ids);
        int getActionId(dist information_set_ids, vector<int> action_ids);

    private:
        static const char* DEFAULT_FILE;
        static const char FILE_DELIM;
        static const int ITERATIONS;
        GameAbstraction* game;
        bool itree;

        double current_regret_sum;
        map<int, double> regrets;
        Smap strategy;
        Smap R;
        Smap S;
        Smap newR;

        int getActionId(dist action_dist, double prob_sum = 1.0);
        bool loadFromFile(const char* filename);
        void saveToFile(const char* filename);

        double recomputeStrategy(Smap &r);
        void computeVanillaCfr(int iterations);
        utility walkTree(long double probs[3]);
};

#endif

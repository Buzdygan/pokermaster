#ifndef CFR_H
#define CFR_H

#include <vector>
#include <map>
#include <cstdlib>
#include "GameAbstraction.h"
using namespace std;

typedef map<pair<int, int>, double> Smap;
typedef map<pair<int, int>, double>::iterator Sit;

class Cfr
{
    public:
        Cfr(GameAbstraction* game, int iterations=ITERATIONS, const char* strategy_file=DEFAULT_FILE, bool samp=true);
        int getActionId(int information_set_id, vector<int> action_ids);
        int getActionId(dist information_set_ids, vector<int> action_ids);

    private:
        static const char* DEFAULT_FILE;
        static const char FILE_DELIM;
        static const int ITERATIONS;
        GameAbstraction* game;
        bool initialized;
        bool sampled;

        double current_regret_sum;
        map<int, double> regrets;
        map<pair<int, int>, utility > action_utility;
        map<int, utility> is_utility;
        Smap strategy;
        Smap R;
        Smap S;
        Smap newR;

        int getActionId(dist action_dist);
        bool loadFromFile(const char* filename);
        void saveToFile(const char* filename);

        double recomputeStrategy(Smap &r);
        void computeVanillaCfr(int iterations);
        utility walkTree(double probs[3], int prev_is_id=-1);
};

#endif

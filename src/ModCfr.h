#ifndef MOD_CFR_H
#define MOD_CFR_H

#include <vector>
#include <map>
#include <set>
#include <cstdlib>
#include "GameAbstraction.h"
using namespace std;

typedef map<pair<int, int>, double> Smap;
typedef map<pair<int, int>, double>::iterator Sit;
typedef vector<pair<pair<int, int>, double> > Nlist; // neighbour list

class ModCfr
{
    public:
        ModCfr(GameAbstraction* game, int iterations=ITERATIONS, const char* strategy_file=DEFAULT_FILE);
        int getActionId(int information_set_id, vector<int> action_ids);
        int getActionId(dist information_set_ids, vector<int> action_ids);

    private:
        static const char* DEFAULT_FILE;
        static const char FILE_DELIM;
        static const int ITERATIONS;
        GameAbstraction* game;

        Smap strategy;
        Smap R;
        Smap S;

        // stable part of graph
        vector<int> all_isets;
        vector<int> isets_topo_ordered;
        map<int, int> in_edges;
        map<int, bool> is_final;
        map<int, int> is_player;
        map<int, Nlist> is_graph;
        int start_is;

        // variables
        map<int, bool> visited;
        map<int, utility> is_utility;
        map<int, double> probs [4]; // 0 - prob[0], 1 - prob[1], 2 - prob[0] * prob[2], 3 - prob[1] * prob[2]

        void _recomputeRegrets();
        void _probsBfs();
        int _exploreTree();
        void _topo_order_isets();
        utility _walkTree(int is_id);
        double _recomputeStrategy(Smap &r);
        int getActionId(dist action_dist);
        bool loadFromFile(const char* filename);
        void saveToFile(const char* filename);
};

#endif

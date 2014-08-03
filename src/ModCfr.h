#ifndef MOD_CFR_H
#define MOD_CFR_H

#include <vector>
#include <map>
#include <set>
#include <cstdlib>
#include "HoldemPokerModAbstraction.h"
using namespace std;

const int MAX_STATES = 18000;
const int MAX_ISETS = 3000;

typedef map<pair<int, int>, double> iSmap;
typedef map<pair<int, int>, double>::iterator iSit;
typedef vector<pair<pair<int, int>, double> > Nlist; // neighbour list

class ModCfr
{
    public:
        ModCfr(HoldemPokerModAbstraction* game, int iterations=ITERATIONS, const char* strategy_file=DEFAULT_FILE);
        int getActionId(int information_set_id, vector<int> action_ids);
        int getActionId(dist information_set_ids, vector<int> action_ids);

    private:
        static const char* DEFAULT_FILE;
        static const char FILE_DELIM;
        static const int ITERATIONS;
        HoldemPokerModAbstraction* game;

        iSmap strategy;

        // stable part of graph
        vector<int> all_states;
        vector<int> player_isets;
        vector<int> states_topo_ordered;
        map<int, int> in_edges;

        int start_state;


        void _recomputeRegrets();
        void _probsBfs();
        int _exploreTree();
        void _topo_order_states();
        utility _walkTree(int is_id);
        double _recomputeStrategy(double t[MAX_ISETS][5]);
        void _copyStrategy();
        int getActionId(dist action_dist);
        bool loadFromFile(const char* filename);
        void saveToFile(const char* filename);
};

#endif

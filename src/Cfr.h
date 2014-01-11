#ifndef CFR_H
#define CFR_H

#include <vector>
#include <map>
#include <cstdlib>
#include "GameAbstraction.h"
using namespace std;

class Cfr
{
    public:
        Cfr(GameAbstraction* game);
        int getActionId(int information_set_id);

    private:
        static const char* DEFAULT_FILE;
        static const char FILE_DELIM;
        GameAbstraction* game;
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

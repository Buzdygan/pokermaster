#ifndef UTILS_H
#define UTILS_H

#include <vector>
using namespace std;

typedef vector<pair<int, double> > dist;

int other(int player_number);
void printCard(int card);
void printHand(vector<int> cards);


class HandEvaluator
{
    public:
        HandEvaluator();
        int evaluateHand(vector<int> cards);
};


#endif

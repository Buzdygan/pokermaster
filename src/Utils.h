#ifndef UTILS_H
#define UTILS_H

#include <vector>
using namespace std;

const int DEFAULT_INITIAL_CASH = 100;
const int SMALL_BLIND = 4;
const int BIG_BLIND = 8;
const int CARDS_NUMBER = 52;
const int LAST_PHASE_NUMBER = 3;
const int DRAW = -1;
const int MAX_GAME_CARDS = 9;

const int GAME_IN_PROGRESS = -1;

const int PHASE_TABLE_CARDS[4] = {0, 3, 1, 1};

int evaluateCards(vector<int> cards);

int other(int player_number);

class HandEvaluator
{
    public:
        HandEvaluator();
        int evaluateHand(vector<int> cards);
    private:
        int HR[32487834];
};

void printCard(int card);
void printHand(vector<int> cards);


#endif

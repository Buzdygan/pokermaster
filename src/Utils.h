#ifndef UTILS_H
#define UTILS_H

#include <vector>
using namespace std;

const int DEFAULT_INITIAL_CASH = 100;
const int DEFAULT_ROUNDS_NUMBER = 1;
const int SMALL_BLIND = 4;
const int BIG_BLIND = 8;
const int CARDS_NUMBER = 6;
const int LAST_PHASE_NUMBER = 1;
const int DRAW = -1;
const int GAME_DRAWN = 6;
const int MAX_GAME_CARDS = 3;
const int MAX_STAKE = 5;
const int FINAL_RANDOM_PHASE = 2;
const int FINAL_BIDDING_PHASE = 1;
const int MAX_BIDS_NUMBER = 2;

const int GAME_IN_PROGRESS = 5;

const int PHASE_TABLE_CARDS[2][2][2] = {{{0,1}, {2,3}}, {{1,2}, {2,3}}};

int other(int player_number);
int evaluateHand(vector<int> cards);

class HandEvaluator
{
    public:
        HandEvaluator();
};

void printCard(int card);
void printHand(vector<int> cards);


#endif

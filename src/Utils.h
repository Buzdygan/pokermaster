#ifndef UTILS_H
#define UTILS_H

#include <vector>
using namespace std;

const int MAX_BASKETS_NUMBER = 15;
const int ONE_CARD_CODES = 52;
const int TWO_CARD_CODES = 1326;
const int THREE_CARD_CODES = 22100;


int other(int player_number);
void printCard(int card);
void printHand(vector<int> cards);


class HandEvaluator
{
    public:
        HandEvaluator(int basket_sizes[4]);
        HandEvaluator();
        int evaluateHand(vector<int> cards);
        int getNextBasket(int stage, int current, int cards_code);
        int getBasketsNumber(int stage);
        int cardsCode(vector<int> cards);
        bool loadTransitions();
        void saveTransitions();
        void computeTransitions();
        void test();
    private:
        int* basket_sizes;
        void _computeCC();
};


#endif

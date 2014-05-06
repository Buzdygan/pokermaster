#ifndef __BASKET_MANAGER_H
#define __BASKET_MANAGER_H

#include <vector>
#include "Utils.h"
using namespace std;

const int MAX_BASKETS_NUMBER = 15;
const int ONE_CARD_CODES = 52;
const int TWO_CARD_CODES = 1326;
const int THREE_CARD_CODES = 22100;

class BasketManager
{
    public:
        BasketManager(int basket_sizes[4]);
        BasketManager();
        int getNextBasket(int stage, int current, int cards_code);
        int getBasketsNumber(int stage);
        int cardsCode(vector<int> cards);
        dist getBasketPairsDistribution(int stage, int basket0, int basket1);
        bool loadTransitions();
        void saveTransitions();
        void computeTransitions();
        void test();
    private:
        int* basket_sizes;
        void _computeCC();
        void _computeBasketsDistribution();
        dist default_distribution[4];
};

#endif

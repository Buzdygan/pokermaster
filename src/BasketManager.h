#ifndef __BASKET_MANAGER_H
#define __BASKET_MANAGER_H

#include <vector>
#include "Utils.h"
using namespace std;

const int MAX_BASKETS_NUMBER = 10;
const int ONE_CARD_CODES = 52;
const int TWO_CARD_CODES = 3000;
const int THREE_CARD_CODES = 22100;

pair<int, int> decode_basket_pair(int action_id);
int encode_basket_pair(int basket0, int basket1);

class BasketManager
{
    public:
        BasketManager(int basket_sizes[4]);
        BasketManager();
        int getNextBasket(int stage, int current, int cards_code);
        int getBasketsNumber(int stage);
        int cardsCode(vector<int> cards);
        dist getBasketPairsDistribution(int stage, int basket0, int basket1);
        void test();
    private:
        int* basket_sizes;
        bool _loadTransitions();
        void _saveTransitions();
        bool _loadDistribution();
        void _saveDistribution();
        void _computeTransitions();
        void _init();
        void _computeCC();
        void _computeBasketsDistribution();
};

#endif

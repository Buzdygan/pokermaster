#ifndef __BASKET_MANAGER_H
#define __BASKET_MANAGER_H

#include <vector>
#include "Utils.h"
using namespace std;

const int FIGS = 13;
const int MAX_BASKETS_NUMBER = 10;
const int ONE_CARD_CODES = 52;
const int TWO_CARD_CODES = 3000;
const int THREE_CARD_CODES = 22100;

const int DEFAULT_BASKET_SIZES [4] = {5, 5, 5, 5};

pair<int, int> decode_basket_pair(int action_id);
int encode_basket_pair(int basket0, int basket1);

class BasketManager
{
    public:
        BasketManager(int basket_sizes[4], HandEvaluator*);
        BasketManager(HandEvaluator*);
        int getNextBasket(int stage, int current, int cards_code);
        int getBasketsNumber(int stage);
        int cardsCode(vector<int> cards);
        dist getBasketPairsDistribution(int stage, int basket0, int basket1);
        void test();
    private:
        HandEvaluator* evaluator;
        int* basket_sizes;
        int _cardsCode(int c0, int c1);
        int _cardsCode(int c0, int c1, int c2);
        bool _loadTransitions();
        void _saveTransitions();
        bool _loadDistribution();
        void _saveDistribution();
        void _computeTransitions();
        void _init();
        void _computeCC();
        void _computeBasketsDistribution();
        void _computeFirstBasket();
        int _computeFirstBasket(int c1, int c2);
        int _determineBasket(int stage, double win_prob);
        int _evaluateCards(int p1, int p2, int o1, int o2, int t1, int t2, int t3, int t4=0, int t5=0);
};

#endif

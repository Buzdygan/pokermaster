#ifndef __BASKET_MANAGER_H
#define __BASKET_MANAGER_H

#include <vector>
#include "Utils.h"
using namespace std;

const int FIGS = 13;
const int DFIGS = 26;
//const int FIGS = 6;
//const int DFIGS = 12;
const int MAX_BASKETS_NUMBER = 10;
const int ONE_CARD_CODES = 52;
const int TWO_CARD_CODES = 3000;
const int THREE_CARD_CODES = 22100;
const int BEHIND = 0;
const int TIED = 1;
const int AHEAD = 2;

const int DEFAULT_BASKET_SIZES [4] = {5, 5, 5, 5};

pair<int, int> decode_basket_pair(int action_id);
int encode_basket_pair(int basket0, int basket1);

struct TP
{
    int a, b, c;
    TP(int x, int y, int z=0)
    {
        a = x;
        b = y;
        c = z;
    };
    int p1(){ return a; };
    int p2(){ return b; };
    int p3(){ return c; };
};

class BasketManager
{
    public:
        BasketManager(int basket_sizes[4], HandEvaluator*);
        BasketManager(HandEvaluator*);
        int getNextBasket(int stage, int current, int cards_code);
        int getBasket(int stage, vector<int> cards);
        int getBasketsNumber(int stage);
        int cardsCode(vector<int> cards);
        dist getBasketPairsDistribution(int stage, int basket0, int basket1);
        void test();
    private:
        HandEvaluator* evaluator;
        int* basket_sizes;
        int _cardsCode(int c0, int c1);
        int _cardsCode(int c0, int c1, int c2);
        int _cardsCode2(int c0, int c1, int c2);
        bool _loadEHS();
        void _saveEHS();
        bool _loadTransitions();
        void _saveTransitions();
        bool _loadDistribution();
        void _saveDistribution();
        void _computeCardCodesMap();
        void _computeCardCombinations();
        void _computeEHS();
        void _computeEHSDistribution();
        void _computeTransitions();
        void _init();
        void _computeCC();
        dist _normalizeDistribution(dist d);
        void _computeBasketsDistribution();
        int _computeBasket(int stage, int* F, int pc1, int pc2, int tc1=0, int tc2=0, int tc3=0, int tc4=0, int tc5=0);
        int _determineBasket(int stage, double ehs);
        int _evaluateCards(int p1, int p2, int o1, int o2, int t1, int t2, int t3, int t4=0, int t5=0);
        double _EHS(int* F, int pc1, int pc2, int tc1=0, int tc2=0, int tc3=0, int tc4=0, int tc5=0);
        int _computePotential(int* HP, int* F, int pc1, int pc2, int oc1, int oc2, int tc1, int tc2, int tc3, int tc4, int tc5);
        void _normalizeBasketTransitions();
};

#endif

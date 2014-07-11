#include<algorithm>
#include<vector>

#include "Utils.h"
#include "BasketManager.h"

const char* TRANSITIONS_FILENAME = "basket_transitions.stg";
const char* DISTRIBUTION_FILENAME = "basket_distribution.stg";

int CC[55][55];
int PF[TWO_CARD_CODES + 5]; // pre flop basket
double FL[MAX_BASKETS_NUMBER][THREE_CARD_CODES + 10][MAX_BASKETS_NUMBER]; // flop basket transistions
double TR[2][MAX_BASKETS_NUMBER][ONE_CARD_CODES + 5][MAX_BASKETS_NUMBER]; // turn and river basket transitions

dist BASKET_DISTRIBUTION[5][MAX_BASKETS_NUMBER * MAX_BASKETS_NUMBER];

pair<int, int> decode_basket_pair(int action_id)
{
    return make_pair(action_id % MAX_BASKETS_NUMBER, action_id / MAX_BASKETS_NUMBER);
}

int encode_basket_pair(int basket0, int basket1)
{
    return basket0 + basket1 * MAX_BASKETS_NUMBER;
}

BasketManager::BasketManager(int bs[4], HandEvaluator* ev)
{
    evaluator = ev;
    basket_sizes = new int[4];
    for (int b = 0; b < 4; b++)
        basket_sizes[b] = bs[b];
    _init();
}

BasketManager::BasketManager(HandEvaluator* ev)
{
    evaluator = ev;
    basket_sizes = new int[4];
    for (int b = 0; b < 4; b++)
        basket_sizes[b] = DEFAULT_BASKET_SIZES[b];
    _init();
}

void BasketManager::_init()
{
    _computeCC();
    if (!_loadTransitions())
    {
        _computeTransitions();
        _saveTransitions();
        _computeBasketsDistribution();
        _saveDistribution();
    }
    else
    {
        if(!_loadDistribution())
        {
            _computeBasketsDistribution();
            _saveDistribution();
        }
    }
}

void BasketManager::_computeBasketsDistribution()
{
    const int M = MAX_BASKETS_NUMBER * MAX_BASKETS_NUMBER;
    int DIST[M];
    memset(DIST, 0, M);
    int n = ONE_CARD_CODES;

    // stage 0 distribution
    int sum = 0;
    for (int c0 = 0; c0 < n; c0++)
        for (int c1 = 0; c1 < n; c1++)
            if (c1 != c0)
                for (int c2 = 0; c2 < n; c2++)
                    if (c2 != c0 && c2 != c1)
                        for (int c3 = 0; c3 < n; c3++)
                            if (c3 != c0 && c3 != c1 && c3 != c2)
                            {
                                vector<int> cards0, cards1;
                                cards0.push_back(c0);
                                cards0.push_back(c1);
                                cards1.push_back(c2);
                                cards1.push_back(c3);
                                int b0 = getNextBasket(0, 0, cardsCode(cards0));
                                int b1 = getNextBasket(0, 0, cardsCode(cards1));
                                DIST[encode_basket_pair(b0, b1)] ++;
                                sum ++;
                            }
    dist distribution0;
    for (int b0 = 0; b0 < basket_sizes[0]; b0++)
    {
        for(int b1 = 0; b1 < basket_sizes[0]; b1++)
        {
            int bpair = encode_basket_pair(b0, b1);
            if (DIST[bpair] > 0)
                distribution0.push_back(make_pair(bpair, double(DIST[bpair]) / sum));
        }
    }
    BASKET_DISTRIBUTION[0][0] = distribution0;

    double DD[M][M];
    double SUM[M];
    memset(DD, 0, M * M);
    memset(SUM, 0, M);

    // stage 1 distribution
    for (int c0 = 0; c0 < n; c0++)
        for (int c1 = 0; c1 < n; c1++)
            if (c1 != c0)
                for (int c2 = 0; c2 < n; c2++)
                    if (c2 != c0 && c2 != c1)
                    {
                        vector<int> cards;
                        cards.push_back(c0);
                        cards.push_back(c1);
                        cards.push_back(c2);
                        int cards_code = cardsCode(cards);
                        for (int a0 = 0; a0 < basket_sizes[0]; a0++)
                            for (int a1 = 0; a1 < basket_sizes[0]; a1++)
                                for (int b0 = 0; b0 < basket_sizes[1]; b0++)
                                    for (int b1 = 0; b1 < basket_sizes[1]; b1++)
                                    {
                                        int apair = encode_basket_pair(a0, a1);
                                        int bpair = encode_basket_pair(b0, b1);
                                        double prob = FL[a0][cards_code][b0] * FL[a1][cards_code][b1];
                                        SUM[apair] += prob;
                                        DD[apair][bpair] += prob;
                                    }
                    }
    for (int a0 = 0; a0 < basket_sizes[0]; a0++)
        for (int a1 = 0; a1 < basket_sizes[0]; a1++)
            for (int b0 = 0; b0 < basket_sizes[1]; b0++)
                for (int b1 = 0; b1 < basket_sizes[1]; b1++)
                {
                    int apair = encode_basket_pair(a0, a1);
                    int bpair = encode_basket_pair(b0, b1);
                    if (SUM[apair] > 0.0 && DD[apair][bpair] > 0.0)
                    {
                        double prob = DD[apair][bpair] / SUM[apair];
                        BASKET_DISTRIBUTION[1][apair].push_back(make_pair(bpair, prob));
                    }
                }

    for(int stage = 2; stage < 4; stage ++)
    {
        memset(DD, 0, M * M);
        memset(SUM, 0, M);
        for (int c = 0; c < n; c++)
            for (int a0 = 0; a0 < basket_sizes[stage - 1]; a0++)
                for (int a1 = 0; a1 < basket_sizes[stage - 1]; a1++)
                    for (int b0 = 0; b0 < basket_sizes[stage]; b0++)
                        for (int b1 = 0; b1 < basket_sizes[stage]; b1++)
                        {
                            int apair = encode_basket_pair(a0, a1);
                            int bpair = encode_basket_pair(b0, b1);
                            double prob = TR[stage - 2][a0][c][b0] * TR[stage-2][a1][c][b1];
                            SUM[apair] += prob;
                            DD[apair][bpair] += prob;
                        }
        for (int a0 = 0; a0 < basket_sizes[stage - 1]; a0++)
            for (int a1 = 0; a1 < basket_sizes[stage - 1]; a1++)
                for (int b0 = 0; b0 < basket_sizes[stage]; b0++)
                    for (int b1 = 0; b1 < basket_sizes[stage]; b1++)
                    {
                        int apair = encode_basket_pair(a0, a1);
                        int bpair = encode_basket_pair(b0, b1);
                        if (SUM[apair] > 0.0 && DD[apair][bpair] > 0.0)
                        {
                            double prob = DD[apair][bpair] / SUM[apair];
                            BASKET_DISTRIBUTION[stage][apair].push_back(make_pair(bpair, prob));
                        }
                    }
        }
}

void BasketManager::_saveDistribution()
{
    FILE *f = fopen(DISTRIBUTION_FILENAME, "w");
    dist d = BASKET_DISTRIBUTION[0][0];
    fprintf(f, "%d\n", (int) d.size());
    for (int i = 0; i < d.size(); i++)
        fprintf(f, "%d %lf\n", d[i].first, d[i].second);
    for (int stage = 1; stage < 4; stage++)
    {
        fprintf(f, "%d\n", basket_sizes[stage] * basket_sizes[stage]);
        for (int b0 = 0; b0 < basket_sizes[stage]; b0++)
            for (int b1 = 0; b1 < basket_sizes[stage]; b1++)
            {
                fprintf(f, "%d %d\n", b0, b1);
                dist d = BASKET_DISTRIBUTION[stage][encode_basket_pair(b0, b1)];
                fprintf(f, "%d\n", (int) d.size());
                for (int i = 0; i < d.size(); i++)
                    fprintf(f, "%d %lf\n", d[i].first, d[i].second);
            }
    }
    fclose(f);
}

bool BasketManager::_loadDistribution()
{
    try
    {
        FILE *f = fopen(DISTRIBUTION_FILENAME, "r");
        int n, bnum;
        double prob;
        // read distribution size
        fscanf(f, "%d\n", &n);
        for (int i = 0; i < n; i++)
        {
            fscanf(f, "%d %lf\n", &bnum, &prob);
            BASKET_DISTRIBUTION[0][0].push_back(make_pair(bnum, prob));
        }
        for (int stage = 1; stage < 4; stage++)
        {
            int m;
            // read basket pairs size
            fscanf(f, "%d\n", &m);
            for (int i = 0; i < m; i++)
            {
                int b0, b1;
                // read basket pair
                fscanf(f, "%d %d\n", &b0, &b1);
                int bpair = encode_basket_pair(b0, b1);
                // read dist size
                fscanf(f, "%d\n", &n);
                for (int j = 0; j < n; j++)
                {
                    fscanf(f, "%d %lf\n", &bnum, &prob);
                    BASKET_DISTRIBUTION[stage][bpair].push_back(make_pair(bnum, prob));
                }
            }
        }
        fclose(f);
    }
    catch (int e)
    {
        return false;
    }
}

dist BasketManager::getBasketPairsDistribution(int stage, int basket0, int basket1)
{
    if (stage == 0)
        return BASKET_DISTRIBUTION[0][0];
    return BASKET_DISTRIBUTION[stage][encode_basket_pair(basket0, basket1)];
}

void BasketManager::_saveTransitions()
{
    FILE *f = fopen(TRANSITIONS_FILENAME, "w");
    fprintf(f, "%d\n", TWO_CARD_CODES);
    for (int i = 0; i < TWO_CARD_CODES; i++)
        fprintf(f, "%d\n", PF[i]);

    fprintf(f, "%d\n", THREE_CARD_CODES * basket_sizes[0] * basket_sizes[1]);
    for (int b1 = 0; b1 < basket_sizes[0]; b1++)
        for (int cc = 0; cc < THREE_CARD_CODES; cc++)
            for (int b2 = 0; b2 < basket_sizes[1]; b2++)
                fprintf(f, "%d %d %d %lf\n", b1, cc, b2, FL[b1][cc][b2]);

    for (int i = 0; i < 2; i++)
    {
        fprintf(f, "%d\n",  ONE_CARD_CODES * basket_sizes[1 + i] * basket_sizes[2 + i]);
        for (int b1 = 0; b1 < basket_sizes[1 + i]; b1++)
            for (int cc = 0; cc < ONE_CARD_CODES; cc++)
                for (int b2 = 0; b2 < basket_sizes[2 + i]; b2++)
                    fprintf(f, "%d %d %d %lf\n", b1, cc, b2, TR[i][b1][cc][b2]);
    }
    fclose(f);
}

bool BasketManager::_loadTransitions()
{
    try {
        FILE *f = fopen(TRANSITIONS_FILENAME, "r");
        if (f == NULL)
            return false;
        int size;
        // reading PF
        fscanf(f, "%d\n", &size);
        for (int i = 0 ; i < size; i++)
        {
            fscanf(f, "%d\n", &PF[i]);
        }
        // reading F
        fscanf(f, "%d\n", &size);
        for (int i = 0 ; i < size; i++)
        {
            int b1, cc, b2;
            double val;
            fscanf(f, "%d %d %d %lf\n", &b1, &cc, &b2, &val);
            FL[b1][cc][b2] = val;
        }
        // reading TR
        for (int j = 0; j < 2; j++)
        {
            fscanf(f, "%d\n", &size);
            for (int i = 0 ; i < size; i++)
            {
                int b1, cc, b2;
                double val;
                fscanf(f, "%d %d %d %lf\n", &b1, &cc, &b2, &val);
                TR[j][b1][cc][b2] = val;
            }
        }
        fclose(f);
        return true;
    }
    catch (int e)
    {
        return false;
    }
}

void BasketManager::_computeTransitions()
{
    // possibilities for the player cards
    vector<pair<TP, vector<int> > > v1;
    for (int p1 = 0; p1 < FIGS; p1 ++)
        for (int p2 = p1 + 1; p2 < FIGS; p2 ++)
        {
            vector<int> cards;
            for (int i = 1; i <= 4; i++)
                for (int j = 1; j <= 4; j++)
                    if (i != j)
                        cards.push_back(_cardsCode(4 * p1 + i, 4 * p2 + j));
            v1.push_back(make_pair(TP (4 * p1 + 2, 4 * p2 + 3), cards));

            vector<int> cards2;
            for (int i = 1; i <= 4; i++)
                cards2.push_back(_cardsCode(4 * p1 + i, 4 * p2 + i));
            v1.push_back(make_pair(TP (4 * p1 + 1, 4 * p2 + 1), cards2));
        }
    for (int p1 = 0; p1 < FIGS; p1 ++)
    {
        vector<int> cards;
        for (int i = 1; i <= 3; i++)
            for (int j = i + 1; j <= 4; j++)
                cards.push_back(_cardsCode(4 * p1 + i, 4 * p1 + j));
        v1.push_back(make_pair(TP (4 * p1 + 2, 4 * p1 + 3), cards));
    }


    // possibilities for the flop cards
    vector<pair<TP, vector<int> > > v2;
    // all the figures different

    for (int t1 = 0; t1 < FIGS; t1 ++)
        for (int t2 = t1 + 1; t2 < FIGS; t2 ++)
            for (int t3 = t2 + 1; t3 < FIGS; t3 ++)
            {
                vector<int> cards;
                for (int i = 1; i <= 4; i++)
                    for (int j = 1; j <= 4; j++)
                        for (int k = 1; k <= 4; k++)
                            if (i != j && i != k && j != k)
                                cards.push_back(_cardsCode(4 * t1 + i, 4 * t2 + j, 4 * t3 + k));
                v2.push_back(make_pair(TP (4 * t1 + 2, 4 * t2 + 3, 4 * t3 + 4), cards));

                vector<int> cards2;
                for (int i = 1; i <= 4; i++)
                    for (int j = 1; j <= 4; j++)
                        if (i != j)
                            cards2.push_back(_cardsCode(4 * t1 + i, 4 * t2 + i, 4 * t3 + j));
                v2.push_back(make_pair(TP (4 * t1 + 1, 4 * t2 + 1, 4 * t3 + 2), cards2));

                vector<int> cards3;
                for (int i = 1; i <= 4; i++)
                    for (int j = 1; j <= 4; j++)
                        if (i != j)
                            cards3.push_back(_cardsCode(4 * t1 + i, 4 * t2 + j, 4 * t3 + i));
                v2.push_back(make_pair(TP (4 * t1 + 1, 4 * t2 + 3, 4 * t3 + 1), cards3));

                vector<int> cards4;
                for (int i = 1; i <= 4; i++)
                    for (int j = 1; j <= 4; j++)
                        if (i != j)
                            cards4.push_back(_cardsCode(4 * t1 + j, 4 * t2 + i, 4 * t3 + i));
                v2.push_back(make_pair(TP (4 * t1 + 4, 4 * t2 + 1, 4 * t3 + 1), cards4));

                vector<int> cards5;
                for (int i = 1; i <= 4; i++)
                    cards5.push_back(_cardsCode(4 * t1 + i, 4 * t2 + i, 4 * t3 + i));
                v2.push_back(make_pair(TP (4 * t1 + 1, 4 * t2 + 1, 4 * t3 + 1), cards5));
            }

    for (int t1 = 0; t1 < FIGS; t1++) // two figures t1
        for (int t2 = 0; t2 < FIGS; t2++) // one figure t2
            if (t1 != t2)
            {
                vector<int> cards;
                // all colors different
                for (int i = 1; i <= 3; i++)
                    for (int j = i+1; j <= 4; j++)
                        for (int k = 1; k <= 4; k++)
                            if (k != i && k != j)
                                cards.push_back(_cardsCode2(4 * t1 + i, 4 * t1 + j, 4 * t2 + k));
                v2.push_back(make_pair(TP (4 * t1 + 2, 4 * t1 + 3, 4 * t2 + 4), cards));

                vector<int> cards2;
                // two colors matching
                for (int i = 1; i <= 4; i++)
                    for (int j = 1; j <= 4; j++)
                        if (i != j)
                            cards2.push_back(_cardsCode2(4 * t1 + i, 4 * t1 + j, 4 * t2 + i));
                v2.push_back(make_pair(TP (4 * t1 + 1, 4 * t1 + 3, 4 * t2 + 1), cards2));
            }
    for (int t1 = 0; t1 < FIGS; t1++) // all figures the same
    {
        vector<int> cards;
        for (int i = 1; i <= 2; i++)
            for (int j = i + 1; j <= 3; j++)
                for (int k = j + 1; k <= 4; k++)
                    cards.push_back(_cardsCode(4 * t1 + i, 4 * t1 + j, 4 * t1 + k));
        v2.push_back(make_pair(TP (4 * t1 + 2, 4 * t1 + 3, 4 * t1 + 4), cards));
    }


    // possibilities for the turn cards (3 stage)
    vector<pair<int, vector<int> > > v3;
    for (int t4 = 0; t4 < FIGS; t4 ++)
    {
        vector<int> cards;
        int c = 4 * t4 + 3;
        for (int i = 1; i <= 4; i++)
            cards.push_back(4 * t4 + i);
        v3.push_back(make_pair(c, cards));
    }

    // possibilities for the river cards (4 stage)
    vector<pair<int, vector<int> > > v4;
    for (int t5 = 0; t5 < FIGS; t5 ++)
    {
        vector<int> cards;
        int c = 4 * t5 + 4;
        for (int i = 1; i <= 4; i++)
            cards.push_back(4 * t5 + i);
        v4.push_back(make_pair(c, cards));
    }

    int F[ONE_CARD_CODES + 3];
    memset(F, 0, sizeof(F));
    for (int i1 = 0; i1 < v1.size(); i1 ++)
    {
        int pc1 = v1[i1].first.p1();
        int pc2 = v1[i1].first.p2();
        F[pc1] ++; F[pc2] ++;
        // determine the basket for this representant
        int basket1 = _computeBasket(1, F, pc1, pc2);
        printf("basket1 : %d\n", basket1);

        // dealing flop cards
        for (int i2 = 0; i2 < v2.size(); i2++)
        {
            int tc1 = v2[i2].first.p1();
            int tc2 = v2[i2].first.p2();
            int tc3 = v2[i2].first.p3();
            if (F[tc1] + F[tc2] + F[tc3] == 0)
            {
                F[tc1] ++; F[tc2] ++; F[tc3] ++;
                int basket2 = _computeBasket(2, F, pc1, pc2, tc1, tc2, tc3);
                //printf("    basket2 : %d\n", basket2);
                for (int i3 = 0; i3 < v3.size(); i3++)
                {
                    int tc4 = v3[i3].first;
                    if (!F[tc4])
                    {
                        F[tc4] ++;
                        int basket3 = _computeBasket(3, F, pc1, pc2, tc1, tc2, tc3, tc4);
                        for (int i4 = 0; i4 < v4.size(); i4++)
                        {
                            int tc5 = v4[i4].first;
                            if (!F[tc5])
                            {
                                F[tc5] ++;
                                int basket4 = _computeBasket(4, F, pc1, pc2, tc1, tc2, tc3, tc4, tc5);
                                // setting basket3 to basket4 transition for all the cards
                                vector<int> cards = v4[i4].second;
                                for (int j4 = 0; j4 < cards.size(); j4 ++)
                                    TR[1][basket3][cards[j4]][basket4] += 1.0;
                                F[tc5] --;
                            }
                        }
                        // setting basket2 to basket3 transition for all the cards
                        vector<int> cards = v3[i3].second;
                        for (int j3 = 0; j3 < cards.size(); j3 ++)
                            TR[0][basket2][cards[j3]][basket3] += 1.0;
                        F[tc4] --;
                    }
                }
                // setting basket1 to basket2 transition for all the cards
                vector<int> card_codes = v2[i2].second;
                for (int j2 = 0; j2 < card_codes.size(); j2 ++)
                {
                    FL[basket1][card_codes[j2]][basket2] += 1.0;
                    //printf("FL[%d][%d][%d] = %lf\n", basket1, code, basket2, FL[basket1][code][basket2]);
                }
                F[tc1] --; F[tc2] --; F[tc3] --;
            }
        }

        // setting basket1 for all the cards
        vector<int> card_codes = v1[i1].second;
        for (int j1 = 0; j1 < card_codes.size(); j1 ++)
        {
            int code = card_codes[j1];
            PF[code] = basket1;
            printf("PF[%d] = %d\n", code, basket1);
        }
        F[pc1] --; F[pc2] --;
    }
    _normalizeBasketTransitions();
}

void BasketManager::_normalizeBasketTransitions()
{
    for (int cc = 0; cc < THREE_CARD_CODES + 2; cc ++)
        for (int b0 = 0; b0 < basket_sizes[0]; b0 ++)
        {
            double sum = 0.0;
            for (int b1 = 0; b1 < basket_sizes[1]; b1 ++)
                sum += FL[b0][cc][b1];
            if (sum > 0.0)
                for (int b1 = 0; b1 < basket_sizes[1]; b1 ++)
                    FL[b0][cc][b1] /= sum;
        }
    for (int ind = 0; ind < 2; ind ++)
        for (int cc = 0; cc < ONE_CARD_CODES + 2; cc ++)
            for (int b1 = 0; b1 < basket_sizes[1 + ind]; b1 ++)
            {
                double sum = 0.0;
                for (int b2 = 0; b2 < basket_sizes[2 + ind]; b2 ++)
                    sum += TR[ind][b1][cc][b2];
                if (sum > 0.0)
                    for (int b2 = 0; b2 < basket_sizes[2 + ind]; b2 ++)
                        TR[ind][b1][cc][b2] /= sum;
            }
}

inline int _inc_vars(int &ahead, int &tied, int &behind, int sc)
{
    ahead += int(sc == AHEAD);
    tied += int(sc == TIED);
    behind += int(sc == BEHIND);
    return sc;
}

inline int _card_map(int lev, int fig)
{
    if (fig < FIGS)
        return fig * 4 + 1;
    else
        return (fig - FIGS) * 4 + lev;
}

int BasketManager::_computeBasket(int stage, int F[ONE_CARD_CODES + 3], int pc1, int pc2, int tc1, int tc2, int tc3, int tc4, int tc5)
{
    return _determineBasket(stage, _EHS(F, pc1, pc2, tc1, tc2, tc3, tc4, tc5));
}

double BasketManager::_EHS(int F[ONE_CARD_CODES + 3], int pc1, int pc2, int tc1, int tc2, int tc3, int tc4, int tc5)
{
    int ahead = 0, tied = 0, behind = 0;
    int HP[3][3];
    memset(HP, 0, sizeof(HP));
    int HPTotal[3] = {0, 0, 0};
    for (int o1 = 0; o1 < DFIGS; o1 ++)
        for (int o2 = o1; o2 < DFIGS; o2 ++)
        {
            int oc1 = _card_map(1, o1), oc2 = _card_map(2, o2);
            if (F[oc1] + F[oc2] == 0)
            {
                // TODO zastanów się czy nie należy zwiększać o inną wartość niż 1
                F[oc1] ++; F[oc2] ++;
                int index = _inc_vars(ahead, tied, behind, _evaluateCards(pc1, pc2, oc1, oc2, tc1, tc2, tc3, tc4, tc5));
                //HPTotal[index] += _computePotential(HP[index], F, pc1, pc2, oc1, oc2, tc1, tc2, tc3, tc4, tc5);
                F[oc1] --; F[oc2] --;
            }
        }

    /*
    printf("BA: %d, BT: %d, BB: %d, TA: %d, TT: %d, TB: %d, AB: %d, AT: %d AA: %d, TOTA: %d, TOTT: %d, TOTB: %d\n",
                                                               HP[BEHIND][AHEAD],
                                                               HP[BEHIND][TIED],
                                                               HP[BEHIND][BEHIND],
                                                               HP[TIED][AHEAD],
                                                               HP[TIED][TIED],
                                                               HP[TIED][BEHIND],
                                                               HP[AHEAD][BEHIND],
                                                               HP[AHEAD][TIED],
                                                               HP[AHEAD][AHEAD],
                                                               HPTotal[AHEAD],
                                                               HPTotal[TIED],
                                                               HPTotal[BEHIND]);
                                                               */
    double ppot = 0.0, npot = 0.0, hs = 0.0;
    if (ahead + tied + behind)
        hs = (double(ahead) + tied / 2.0) / (ahead + tied + behind);
    return hs;
    /*
    if (HPTotal[BEHIND] + HPTotal[TIED])
        ppot = ((double)HP[BEHIND][AHEAD] + HP[BEHIND][TIED]/ 2.0 + HP[TIED][AHEAD] / 2.0) / (double)(HPTotal[BEHIND] + HPTotal[TIED]);
    if (HPTotal[AHEAD] + HPTotal[TIED])
        npot = ((double)HP[AHEAD][BEHIND] + HP[TIED][BEHIND] / 2.0 + HP[AHEAD][TIED] / 2.0) / (double)(HPTotal[AHEAD] + HPTotal[TIED]);
    double score = hs * (1.0 - npot) + (1.0 - hs) * ppot;
    //printf("hs: %lf ppot: %lf npot: %lf score: %lf\n", hs, ppot, npot, score);
    return score;
    */
}

int BasketManager::_computePotential(int HP[3], int F[ONE_CARD_CODES+ 3], int pc1, int pc2, int oc1,
                                      int oc2, int tc1, int tc2, int tc3, int tc4, int tc5)
{
    int total = 0;
    if (!tc1)
    {
        for (int t1 = 0; t1 < DFIGS; t1++)
            for (int t2 = t1; t2 < DFIGS; t2++)
                for (int t3 = t2; t3 < DFIGS; t3++)
                    for (int t4 = 0; t4 < DFIGS; t4++)
                        for (int t5 = 0; t5 < DFIGS; t5++)
                        {
                            tc1 = _card_map(1, t1);
                            tc2 = _card_map(2, t2);
                            tc3 = _card_map(3, t3);
                            tc4 = _card_map(1, t4);
                            tc5 = _card_map(2, t5);
                            if (!(F[tc1] + F[tc2] + F[tc3] + F[tc4] + F[tc5]))
                            {
                                HP[_evaluateCards(pc1, pc2, oc1, oc2, tc1, tc2, tc3, tc4, tc5)] ++;
                                total ++;
                            }
                        }
    }
    else if (!tc4)
    {
        for (int t4 = 0; t4 < DFIGS; t4++)
            for (int t5 = 0; t5 < DFIGS; t5++)
            {
                tc4 = _card_map(1, t4); tc5 = _card_map(2, t5);
                if (!(F[tc4] + F[tc5]))
                {
                    HP[_evaluateCards(pc1, pc2, oc1, oc2, tc1, tc2, tc3, tc4, tc5)] ++;
                    total ++;
                }
            }
    }
    else if (!tc5)
    {
        for (int t5 = 0; t5 < DFIGS; t5++)
        {
            tc5 = _card_map(2, t5);
            if (!F[tc5])
            {
                HP[_evaluateCards(pc1, pc2, oc1, oc2, tc1, tc2, tc3, tc4, tc5)] ++;
                total ++;
            }
        }
    }
    else
    {
        HP[_evaluateCards(pc1, pc2, oc1, oc2, tc1, tc2, tc3, tc4, tc5)] ++;
        total ++;
    }
    return total;
}

int BasketManager::_evaluateCards(int p1, int p2, int o1, int o2, int t1, int t2, int t3, int t4, int t5)
{
    int pscore = evaluator -> evaluateHand(p1, p2, t1, t2, t3, t4, t5);
    int oppscore = evaluator -> evaluateHand(o1, o2, t1, t2, t3, t4, t5);
    //printf("p1: %d, p2: %d, score: %d\n", p1, p2, pscore);
    //printf("o1: %d, o2: %d, score: %d\n", o1, o2, oppscore);
    if (pscore > oppscore)
        return AHEAD;
    if (pscore == oppscore)
        return TIED;
    if (pscore < oppscore)
        return BEHIND;
}

/* returns basket based on the stage and probability of win */
int BasketManager::_determineBasket(int stage, double ehs)
{
    int res = min(int(ehs * basket_sizes[stage - 1]), basket_sizes[stage-1] - 1);
    return res;
}

int BasketManager::cardsCode(vector<int> cards)
{
    int n = cards.size();
    if (n == 1)
        return cards[0];
    sort(cards.begin(), cards.end());
    if (n == 2)
        return cards[0] + cards[1] * 53;
    return CC[cards[0]][cards[1]] + cards[2] - cards[1] - 1;
}

int BasketManager::_cardsCode(int c0, int c1)
{
    return c0 < c1 ? c0 + c1 * 53 : c1 + c0 * 53;
}

/* cards should be given in increasing order */
int BasketManager::_cardsCode(int c0, int c1, int c2)
{
    return CC[c0][c1] + c2 - c1 - 1;
}

/* no restrictions on order of the cards */
int BasketManager::_cardsCode2(int c0, int c1, int c2)
{
    vector<int> cards;
    cards.push_back(c0);
    cards.push_back(c1);
    cards.push_back(c2);
    return cardsCode(cards);
}

int BasketManager::getBasketsNumber(int stage)
{
    return basket_sizes[stage];
}

int BasketManager::getNextBasket(int stage, int current, int cards_code)
{
    log(1, "stage: %d, current: %d, cards_code: %d\n", stage, current, cards_code);
    if (stage == 0) // pre flop
        return PF[cards_code];

    double *dist;
    if (stage == 1)
        dist = FL[current][cards_code];
    if (stage > 1)
        dist = TR[stage - 2][current][cards_code];

    int n = basket_sizes[stage]; // number of baskets in this stage
    double r = (double) rand() / RAND_MAX;
    double sum = 0.0;
    for (int i = 0; i < n; i ++)
    {
        sum += dist[i];
        if (r < sum)
            return i;
    }
    return 0;
}

void BasketManager::test()
{
    int c = 0;
    vector<int> v;
    while (c >= 0)
    {
        scanf("%d", &c);
        if (c == 0)
        {
            printf("value: %d\n", cardsCode(v));
            v.clear();
        }
        else
            v.push_back(c);
    }
}

void BasketManager::_computeCC()
{
    int cnt = 0;
    for (int i = 1; i <= 52; i++)
        for (int j = i+1; j <= 52; j++)
        {
            CC[i][j] = cnt;
            cnt += 52 - j;
        }
}

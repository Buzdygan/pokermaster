#include <algorithm>
#include <vector>
#include <map>
#include <cstring>
#include <fstream>

#include "Utils.h"
#include "BasketManager.h"

const char* EHS_FILENAME = "ehs_pot.cpt";

const int EHS_SIZE1 = 169;
const int EHS_SIZE2 = 1755;
const int EHS_SIZE3 = 13;
const int EHS_SIZE4 = 13;

int CC[55][55];
int PF[2][TWO_CARD_CODES + 5]; // pre flop basket
double FL[2][MAX_BASKETS_NUMBER][THREE_CARD_CODES + 10][MAX_BASKETS_NUMBER]; // flop basket transistions
double TR[2][2][MAX_BASKETS_NUMBER][ONE_CARD_CODES + 5][MAX_BASKETS_NUMBER]; // turn and river basket transitions

double EHS1[EHS_SIZE1];
double EHS2[EHS_SIZE2][EHS_SIZE1];
double EHS3[EHS_SIZE3][EHS_SIZE2][EHS_SIZE1];
double EHS4[EHS_SIZE4][EHS_SIZE3][EHS_SIZE2][EHS_SIZE1];
int EHS_DIST[4][102];

int B1[2][EHS_SIZE1];
int B2[2][EHS_SIZE2][EHS_SIZE1];
int B3[2][EHS_SIZE3][EHS_SIZE2][EHS_SIZE1];
int B4[2][EHS_SIZE4][EHS_SIZE3][EHS_SIZE2][EHS_SIZE1];



// thresholds for the given baskets, THR[2][0] = 0.3 means that ehs < 0.3 in round 2 => basket0
double THR[4][MAX_BASKETS_NUMBER + 2];
map<int, int> CARD_CODES_MAP[4];

dist BASKET_DISTRIBUTION[2][5][MAX_BASKETS_NUMBER * MAX_BASKETS_NUMBER];

bool card_combinations_computed = false;
int S1[EHS_SIZE1];
int S2[EHS_SIZE2];
int S3[EHS_SIZE3];
int S4[EHS_SIZE4];

// possibilities for the player cards
vector<pair<TP, vector<int> > > v1;
// possibilities for the flop cards
vector<pair<TP, vector<int> > > v2;
// possibilities for the turn cards (3 stage)
vector<pair<int, vector<int> > > v3;
// possibilities for the river cards (4 stage)
vector<pair<int, vector<int> > > v4;

pair<int, int> decode_basket_pair(int action_id)
{
    return make_pair(action_id % MAX_BASKETS_NUMBER, action_id / MAX_BASKETS_NUMBER);
}

int encode_basket_pair(int basket0, int basket1)
{
    return basket0 + basket1 * MAX_BASKETS_NUMBER;
}

BasketManager::BasketManager(int ind, int bs[4], HandEvaluator* ev)
{
    index = ind;
    evaluator = ev;
    basket_sizes = new int[4];
    for (int b = 0; b < 4; b++)
        basket_sizes[b] = bs[b];
    _init();
}

BasketManager::BasketManager(int ind, HandEvaluator* ev)
{
    index = ind;
    evaluator = ev;
    basket_sizes = new int[4];
    for (int b = 0; b < 4; b++)
        basket_sizes[b] = DEFAULT_BASKET_SIZES[b];
    _init();
}

void BasketManager::_init()
{
    sprintf(TRANSITIONS_FILENAME, "basket_transitions-%d-%d-%d-%d.stg", basket_sizes[0], basket_sizes[1], basket_sizes[2], basket_sizes[3]);
    sprintf(DISTRIBUTION_FILENAME, "basket_distribution-%d-%d-%d-%d.stg", basket_sizes[0], basket_sizes[1], basket_sizes[2], basket_sizes[3]);
    _computeCC();
    if (!_loadTransitions())
    {
        printf("computing Transitions\n");
        _computeTransitions();
        _saveTransitions();
        _computeBasketsDistribution();
        _saveDistribution();
    }
    else
    {
        if(!_loadEHS())
        {
            printf("computing EHS\n");
            _computeCardCombinations();
            _computeEHS();
        }
        if(!_loadDistribution())
        {
            printf("computing Distribution\n");
            _computeBasketsDistribution();
            _saveDistribution();
        }
    }
    _computeEHSDistribution();
}

dist BasketManager::_normalizeDistribution(dist d)
{
    dist res;
    double sum = 0.0;
    for (int i = 0; i < d.size(); i++)
        sum += d[i].second;
    if (sum > 0.0)
        for (int i = 0; i < d.size(); i++)
            res.push_back(make_pair(d[i].first, (double)d[i].second / sum));
    return res;
}

void BasketManager::_computeBasketsDistribution()
{
    const int M = MAX_BASKETS_NUMBER * MAX_BASKETS_NUMBER;
    int DIST[M];
    memset(DIST, 0, sizeof(DIST));
    int n = ONE_CARD_CODES;

    // stage 0 distribution
    for (int c0 = 1; c0 <= n; c0++)
        for (int c1 = c0 + 1; c1 <= n; c1++)
            if (c1 != c0)
                for (int c2 = 1; c2 <= n; c2++)
                    if (c2 != c0 && c2 != c1)
                        for (int c3 = c2 + 1; c3 <= n; c3++)
                            if (c3 != c0 && c3 != c1)
                            {
                                vector<int> cards0, cards1;
                                cards0.push_back(c0);
                                cards0.push_back(c1);
                                cards1.push_back(c2);
                                cards1.push_back(c3);
                                int b0 = getNextBasket(0, 0, _cardsCode(c0, c1));
                                int b1 = getNextBasket(0, 0, _cardsCode(c2, c3));
                                DIST[encode_basket_pair(b0, b1)] ++;
                            }
    dist distribution0;
    for (int b0 = 0; b0 < basket_sizes[0]; b0++)
        for(int b1 = 0; b1 < basket_sizes[0]; b1++)
        {
            int bpair = encode_basket_pair(b0, b1);
            if (DIST[bpair] > 0)
                distribution0.push_back(make_pair(bpair, (double)DIST[bpair]));
        }
    BASKET_DISTRIBUTION[index][0][0] = _normalizeDistribution(distribution0);

    double DD[M][M];
    memset(DD, 0, sizeof(DD));

    // stage 1 distribution
    for (int c0 = 1; c0 <= n - 2; c0++)
        for (int c1 = c0 + 1; c1 <= n - 1; c1++)
            for (int c2 = c1 + 1; c2 <= n; c2++)
            {
                int cards_code = _cardsCode(c0, c1, c2);
                for (int a0 = 0; a0 < basket_sizes[0]; a0++)
                    for (int a1 = 0; a1 < basket_sizes[0]; a1++)
                        for (int b0 = 0; b0 < basket_sizes[1]; b0++)
                            for (int b1 = 0; b1 < basket_sizes[1]; b1++)
                            {
                                int apair = encode_basket_pair(a0, a1);
                                int bpair = encode_basket_pair(b0, b1);
                                double prob = FL[index][a0][cards_code][b0] * FL[index][a1][cards_code][b1];
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
                    if (DD[apair][bpair] > 0.0)
                        BASKET_DISTRIBUTION[index][1][apair].push_back(make_pair(bpair, DD[apair][bpair]));
                }

    for(int stage = 2; stage < 4; stage ++)
    {
        memset(DD, 0, sizeof(DD));
        for (int a0 = 0; a0 < basket_sizes[stage - 1]; a0++)
            for (int a1 = 0; a1 < basket_sizes[stage - 1]; a1++)
            {
                int apair = encode_basket_pair(a0, a1);
                for (int b0 = 0; b0 < basket_sizes[stage]; b0++)
                    for (int b1 = 0; b1 < basket_sizes[stage]; b1++)
                    {
                        int bpair = encode_basket_pair(b0, b1);
                        double prob = 0.0;
                        for (int c = 1; c <= n; c++)
                            prob += TR[index][stage - 2][a0][c][b0] * TR[index][stage-2][a1][c][b1];
                        DD[apair][bpair] = prob;
                    }
                for (int b0 = 0; b0 < basket_sizes[stage]; b0++)
                    for (int b1 = 0; b1 < basket_sizes[stage]; b1++)
                    {
                        int bpair = encode_basket_pair(b0, b1);
                        if (DD[apair][bpair] > 0.0)
                            BASKET_DISTRIBUTION[index][stage][apair].push_back(make_pair(bpair, DD[apair][bpair]));
                    }
            }
    }
    for (int stage = 1; stage < 4; stage ++)
        for (int a0 = 0; a0 < basket_sizes[stage - 1]; a0++)
            for (int a1 = 0; a1 < basket_sizes[stage - 1]; a1++)
            {
                int apair = encode_basket_pair(a0, a1);
                BASKET_DISTRIBUTION[index][stage][apair] = _normalizeDistribution(BASKET_DISTRIBUTION[index][stage][apair]);
            }
}

void BasketManager::_saveDistribution()
{
    FILE *f = fopen(DISTRIBUTION_FILENAME, "w");
    dist d = BASKET_DISTRIBUTION[index][0][0];
    fprintf(f, "%d\n", (int) d.size());
    for (int i = 0; i < d.size(); i++)
        fprintf(f, "%d %lf\n", d[i].first, d[i].second);
    for (int stage = 0; stage < 3; stage++)
    {
        fprintf(f, "%d\n", basket_sizes[stage] * basket_sizes[stage]);
        for (int b0 = 0; b0 < basket_sizes[stage]; b0++)
            for (int b1 = 0; b1 < basket_sizes[stage]; b1++)
            {
                fprintf(f, "%d %d\n", b0, b1);
                dist d = BASKET_DISTRIBUTION[index][stage + 1][encode_basket_pair(b0, b1)];
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
            BASKET_DISTRIBUTION[index][0][0].push_back(make_pair(bnum, prob));
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
                    BASKET_DISTRIBUTION[index][stage][bpair].push_back(make_pair(bnum, prob));
                }
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

dist BasketManager::getBasketPairsDistribution(int stage, int basket0, int basket1)
{
    if (stage == 0)
        return BASKET_DISTRIBUTION[index][0][0];
    return BASKET_DISTRIBUTION[index][stage][encode_basket_pair(basket0, basket1)];
}

void BasketManager::_saveTransitions()
{
    FILE *f = fopen(TRANSITIONS_FILENAME, "w");
    fprintf(f, "%d\n", TWO_CARD_CODES);
    for (int i = 0; i < TWO_CARD_CODES; i++)
        fprintf(f, "%d\n", PF[index][i]);

    fprintf(f, "%d\n", THREE_CARD_CODES * basket_sizes[0] * basket_sizes[1]);
    for (int b1 = 0; b1 < basket_sizes[0]; b1++)
        for (int cc = 0; cc < THREE_CARD_CODES; cc++)
            for (int b2 = 0; b2 < basket_sizes[1]; b2++)
                fprintf(f, "%d %d %d %lf\n", b1, cc, b2, FL[index][b1][cc][b2]);

    for (int i = 0; i < 2; i++)
    {
        fprintf(f, "%d\n",  ONE_CARD_CODES * basket_sizes[1 + i] * basket_sizes[2 + i]);
        for (int b1 = 0; b1 < basket_sizes[1 + i]; b1++)
            for (int cc = 0; cc < ONE_CARD_CODES; cc++)
                for (int b2 = 0; b2 < basket_sizes[2 + i]; b2++)
                    fprintf(f, "%d %d %d %lf\n", b1, cc, b2, TR[index][i][b1][cc][b2]);
    }

    for (int i1 = 0; i1 < EHS_SIZE1; i1++)
        fprintf(f, "%d\n", B1[index][i1]);

    for (int i2 = 0; i2 < EHS_SIZE2; i2++)
        for (int i1 = 0; i1 < EHS_SIZE1; i1++)
            fprintf(f, "%d\n", B2[index][i2][i1]);

    for (int i3 = 0; i3 < EHS_SIZE3; i3++)
        for (int i2 = 0; i2 < EHS_SIZE2; i2++)
            for (int i1 = 0; i1 < EHS_SIZE1; i1++)
                fprintf(f, "%d\n", B3[index][i3][i2][i1]);

    for (int i4 = 0; i4 < EHS_SIZE4; i4++)
        for (int i3 = 0; i3 < EHS_SIZE3; i3++)
            for (int i2 = 0; i2 < EHS_SIZE2; i2++)
                for (int i1 = 0; i1 < EHS_SIZE1; i1++)
                    fprintf(f, "%d\n", B4[index][i4][i3][i2][i1]);

    for (int i = 0; i < EHS_SIZE1; i++)
        fprintf(f, "%d\n", S1[i]);

    for (int i = 0; i < EHS_SIZE2; i++)
        fprintf(f, "%d\n", S2[i]);

    for (int i = 0; i < EHS_SIZE3; i++)
        fprintf(f, "%d\n", S3[i]);

    for (int i = 0; i < EHS_SIZE4; i++)
        fprintf(f, "%d\n", S4[i]);

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
            fscanf(f, "%d\n", &PF[index][i]);
        }
        // reading F
        fscanf(f, "%d\n", &size);
        for (int i = 0 ; i < size; i++)
        {
            int b1, cc, b2;
            double val;
            fscanf(f, "%d %d %d %lf\n", &b1, &cc, &b2, &val);
            FL[index][b1][cc][b2] = val;
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
                TR[index][j][b1][cc][b2] = val;
            }
        }

        // READING B
        for (int i1 = 0; i1 < EHS_SIZE1; i1++)
            fscanf(f, "%d\n", &B1[index][i1]);

        for (int i2 = 0; i2 < EHS_SIZE2; i2++)
            for (int i1 = 0; i1 < EHS_SIZE1; i1++)
                fscanf(f, "%d\n", &B2[index][i2][i1]);

        for (int i3 = 0; i3 < EHS_SIZE3; i3++)
            for (int i2 = 0; i2 < EHS_SIZE2; i2++)
                for (int i1 = 0; i1 < EHS_SIZE1; i1++)
                    fscanf(f, "%d\n", &B3[index][i3][i2][i1]);

        for (int i4 = 0; i4 < EHS_SIZE4; i4++)
            for (int i3 = 0; i3 < EHS_SIZE3; i3++)
                for (int i2 = 0; i2 < EHS_SIZE2; i2++)
                    for (int i1 = 0; i1 < EHS_SIZE1; i1++)
                        fscanf(f, "%d\n", &B4[index][i4][i3][i2][i1]);

        for (int i = 0; i < EHS_SIZE1; i++)
            fscanf(f, "%d\n", &S1[i]);

        for (int i = 0; i < EHS_SIZE2; i++)
            fscanf(f, "%d\n", &S2[i]);

        for (int i = 0; i < EHS_SIZE3; i++)
            fscanf(f, "%d\n", &S3[i]);

        for (int i = 0; i < EHS_SIZE4; i++)
            fscanf(f, "%d\n", &S4[i]);

        fclose(f);
        return true;
    }
    catch (int e)
    {
        return false;
    }
}

void BasketManager::_saveEHS()
{
    FILE *f = fopen(EHS_FILENAME, "w");

    // EHS_DIST
    for (int st = 0; st < 4; st++)
        for (int p = 0; p <= 100; p++)
            fprintf(f, "%d\n", EHS_DIST[st][p]);

    for (int st = 0; st < 4; st++)
    {
        fprintf(f, "%d\n", (int)CARD_CODES_MAP[st].size());
        for (map<int, int>::iterator it = CARD_CODES_MAP[st].begin(); it != CARD_CODES_MAP[st].end(); it ++)
            fprintf(f, "%d %d\n", it -> first, it -> second);
    }

    for (int i1 = 0; i1 < EHS_SIZE1; i1++)
        fprintf(f, "%lf\n", EHS1[i1]);

    for (int i2 = 0; i2 < EHS_SIZE2; i2++)
        for (int i1 = 0; i1 < EHS_SIZE1; i1++)
            fprintf(f, "%lf\n", EHS2[i2][i1]);

    for (int i3 = 0; i3 < EHS_SIZE3; i3++)
        for (int i2 = 0; i2 < EHS_SIZE2; i2++)
            for (int i1 = 0; i1 < EHS_SIZE1; i1++)
                fprintf(f, "%lf\n", EHS3[i3][i2][i1]);

    for (int i4 = 0; i4 < EHS_SIZE4; i4++)
        for (int i3 = 0; i3 < EHS_SIZE3; i3++)
            for (int i2 = 0; i2 < EHS_SIZE2; i2++)
                for (int i1 = 0; i1 < EHS_SIZE1; i1++)
                    fprintf(f, "%lf\n", EHS4[i4][i3][i2][i1]);
    fclose(f);
}

bool BasketManager::_loadEHS()
{
    try
    {
        FILE *f = fopen(EHS_FILENAME, "r");
        if (f == NULL)
            return false;
        // EHS_DIST
        for (int st = 0; st < 4; st++)
            for (int p = 0; p <= 100; p++)
                fscanf(f, "%d\n", &EHS_DIST[st][p]);

        for (int st = 0; st < 4; st++)
        {
            int cc, index, n;
            fscanf(f, "%d\n", &n);
            for (int i = 0; i < n; i++)
            {
                fscanf(f, "%d %d\n", &cc, &index);
                CARD_CODES_MAP[st][cc] = index;
            }
        }

        for (int i1 = 0; i1 < EHS_SIZE1; i1++)
            fscanf(f, "%lf\n", &EHS1[i1]);

        for (int i2 = 0; i2 < EHS_SIZE2; i2++)
            for (int i1 = 0; i1 < EHS_SIZE1; i1++)
                fscanf(f, "%lf\n", &EHS2[i2][i1]);

        for (int i3 = 0; i3 < EHS_SIZE3; i3++)
            for (int i2 = 0; i2 < EHS_SIZE2; i2++)
                for (int i1 = 0; i1 < EHS_SIZE1; i1++)
                    fscanf(f, "%lf\n", &EHS3[i3][i2][i1]);

        for (int i4 = 0; i4 < EHS_SIZE4; i4++)
            for (int i3 = 0; i3 < EHS_SIZE3; i3++)
                for (int i2 = 0; i2 < EHS_SIZE2; i2++)
                    for (int i1 = 0; i1 < EHS_SIZE1; i1++)
                        fscanf(f, "%lf\n", &EHS4[i4][i3][i2][i1]);

        fclose(f);
        return true;
    }
    catch (int e)
    {
        return false;
    }
}

void BasketManager::_computeCardCodesMap()
{
    for (int i = 0; i < v1.size(); i++)
        for (int j = 0; j < v1[i].second.size(); j++)
            CARD_CODES_MAP[0][v1[i].second[j]] = i;
    for (int i = 0; i < v2.size(); i++)
        for (int j = 0; j < v2[i].second.size(); j++)
            CARD_CODES_MAP[1][v2[i].second[j]] = i;
    for (int i = 0; i < v3.size(); i++)
        for (int j = 0; j < v3[i].second.size(); j++)
            CARD_CODES_MAP[2][v3[i].second[j]] = i;
    for (int i = 0; i < v4.size(); i++)
        for (int j = 0; j < v4[i].second.size(); j++)
            CARD_CODES_MAP[3][v4[i].second[j]] = i;
}

void BasketManager::_computeCardCombinations()
{
    if (!card_combinations_computed)
    {
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

        for (int t4 = 0; t4 < FIGS; t4 ++)
        {
            vector<int> cards;
            int c = 4 * t4 + 3;
            for (int i = 1; i <= 4; i++)
                cards.push_back(4 * t4 + i);
            v3.push_back(make_pair(c, cards));
        }

        for (int t5 = 0; t5 < FIGS; t5 ++)
        {
            vector<int> cards;
            int c = 4 * t5 + 4;
            for (int i = 1; i <= 4; i++)
                cards.push_back(4 * t5 + i);
            v4.push_back(make_pair(c, cards));
        }

        for (int i = 0; i < v1.size(); i++)
            S1[i] = v1[i].second.size();
        for (int i = 0; i < v2.size(); i++)
            S2[i] = v2[i].second.size();
        for (int i = 0; i < v3.size(); i++)
            S3[i] = v3[i].second.size();
        for (int i = 0; i < v4.size(); i++)
            S4[i] = v4[i].second.size();
        _computeCardCodesMap();
        card_combinations_computed = true;
    }
}

int _perc(double e)
{
    return int(e * 100.0);
}

void BasketManager::_computeEHS()
{
    int F[ONE_CARD_CODES + 3];
    memset(F, 0, sizeof(F));
    double ehs = 0.0;
    for (int i1 = 0; i1 < v1.size(); i1 ++)
    {
        int pc1 = v1[i1].first.p1();
        int pc2 = v1[i1].first.p2();
        F[pc1] ++; F[pc2] ++;

        printf("Progress: %d / %d\n", i1 + 1, (int)v1.size());

        EHS_DIST[0][_perc(EHS1[i1] =_EHS(F, pc1, pc2))] += S1[i1];
        // dealing flop cards
        for (int i2 = 0; i2 < v2.size(); i2++)
        {
            int tc1 = v2[i2].first.p1();
            int tc2 = v2[i2].first.p2();
            int tc3 = v2[i2].first.p3();
            if (F[tc1] + F[tc2] + F[tc3] == 0)
            {
                F[tc1] ++; F[tc2] ++; F[tc3] ++;
                EHS_DIST[1][_perc(EHS2[i2][i1] = _EHS(F, pc1, pc2, tc1, tc2, tc3))] += S2[i2];
                for (int i3 = 0; i3 < v3.size(); i3++)
                {
                    int tc4 = v3[i3].first;
                    if (!F[tc4])
                    {
                        F[tc4] ++;
                        EHS_DIST[2][_perc(EHS3[i3][i2][i1] = _EHS(F, pc1, pc2, tc1, tc2, tc3, tc4))] += S3[i3];

                        for (int i4 = 0; i4 < v4.size(); i4++)
                        {
                            int tc5 = v4[i4].first;
                            if (!F[tc5])
                            {
                                F[tc5] ++;
                                EHS_DIST[3][_perc(EHS4[i4][i3][i2][i1] = _EHS(F, pc1, pc2, tc1, tc2, tc3, tc4, tc5))] += S4[i4];
                                F[tc5] --;
                            }
                        }
                        F[tc4] --;
                    }
                }
                F[tc1] --; F[tc2] --; F[tc3] --;
            }
        }
        F[pc1] --; F[pc2] --;
    }
    _saveEHS();
}

void BasketManager::_computeEHSDistribution()
{
    for (int st = 0; st < 4; st++)
    {
        int bnum = basket_sizes[st];
        int total = 0;
        for (int p = 0; p <= 100; p++)
            total += EHS_DIST[st][p];

        // computing percentage thresholds for the baskets;
        double thresholds[bnum + 1];
        double th = 0.0;
        for (int b = 0; b < bnum; b++)
        {
            th += 1.0 / bnum;
            thresholds[b] = th;
        }
        thresholds[bnum - 1] = 0.99999;

        int sum = 0;
        int current_basket = 0;
        for (int p = 0; p <= 100; p++)
        {
            sum += EHS_DIST[st][p];
            if ((double)sum / total >= thresholds[current_basket])
            {
                THR[st][current_basket] = (double)p / 100.0;
                printf("st: %d, basket: %d, threshold: %.02lf\n", st, current_basket, p / 100.0);
                current_basket ++;
            }
            if (current_basket >= bnum)
                break;
        }
    }
}

void BasketManager::_computeTransitions()
{
    _computeCardCombinations();
    if(!_loadEHS())
        _computeEHS();
    _computeEHSDistribution();

    int F[ONE_CARD_CODES + 3];
    memset(F, 0, sizeof(F));
    double ehs = 0.0;
    for (int i1 = 0; i1 < v1.size(); i1 ++)
    {
        int pc1 = v1[i1].first.p1();
        int pc2 = v1[i1].first.p2();
        F[pc1] ++; F[pc2] ++;

        printf("Progress: %d / %d\n", i1 + 1, (int)v1.size());
        int basket1 = _determineBasket(0, EHS1[i1]);
        B1[index][i1] = basket1;
        printf("basket1: %d\n", basket1);

        // dealing flop cards
        for (int i2 = 0; i2 < v2.size(); i2++)
        {
            int tc1 = v2[i2].first.p1();
            int tc2 = v2[i2].first.p2();
            int tc3 = v2[i2].first.p3();
            if (F[tc1] + F[tc2] + F[tc3] == 0)
            {
                F[tc1] ++; F[tc2] ++; F[tc3] ++;
                int basket2 = _determineBasket(1, EHS2[i2][i1]);
                B2[index][i2][i1] = basket2;
                for (int i3 = 0; i3 < v3.size(); i3++)
                {
                    int tc4 = v3[i3].first;
                    if (!F[tc4])
                    {
                        F[tc4] ++;
                        int basket3 = _determineBasket(2, EHS3[i3][i2][i1]);
                        B3[index][i3][i2][i1] = basket3;

                        for (int i4 = 0; i4 < v4.size(); i4++)
                        {
                            int tc5 = v4[i4].first;
                            if (!F[tc5])
                            {
                                F[tc5] ++;
                                int basket4 = _determineBasket(3, EHS4[i4][i3][i2][i1]);
                                B4[index][i4][i3][i2][i1] = basket4;
                                // setting basket3 to basket4 transition for all the cards
                                vector<int> cards = v4[i4].second;
                                for (int j4 = 0; j4 < cards.size(); j4 ++)
                                    TR[index][1][basket3][cards[j4]][basket4] += 1.0;
                                F[tc5] --;
                            }
                        }
                        // setting basket2 to basket3 transition for all the cards
                        vector<int> cards = v3[i3].second;
                        for (int j3 = 0; j3 < cards.size(); j3 ++)
                            TR[index][0][basket2][cards[j3]][basket3] += 1.0;
                        F[tc4] --;
                    }
                }
                // setting basket1 to basket2 transition for all the cards
                vector<int> card_codes = v2[i2].second;
                for (int j2 = 0; j2 < card_codes.size(); j2 ++)
                {
                    FL[index][basket1][card_codes[j2]][basket2] += 1.0;
                }
                F[tc1] --; F[tc2] --; F[tc3] --;
            }
        }

        // setting basket1 for all the cards
        vector<int> card_codes = v1[i1].second;
        for (int j1 = 0; j1 < card_codes.size(); j1 ++)
        {
            int code = card_codes[j1];
            PF[index][code] = basket1;
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
                sum += FL[index][b0][cc][b1];
            if (sum > 0.0)
                for (int b1 = 0; b1 < basket_sizes[1]; b1 ++)
                    FL[index][b0][cc][b1] /= sum;
        }
    for (int ind = 0; ind < 2; ind ++)
        for (int cc = 0; cc < ONE_CARD_CODES + 2; cc ++)
            for (int b1 = 0; b1 < basket_sizes[1 + ind]; b1 ++)
            {
                double sum = 0.0;
                for (int b2 = 0; b2 < basket_sizes[2 + ind]; b2 ++)
                    sum += TR[index][ind][b1][cc][b2];
                if (sum > 0.0)
                    for (int b2 = 0; b2 < basket_sizes[2 + ind]; b2 ++)
                        TR[index][ind][b1][cc][b2] /= sum;
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

double BasketManager::_EHS(int F[ONE_CARD_CODES + 3], int pc1, int pc2, int tc1, int tc2, int tc3, int tc4, int tc5)
{
    int ahead = 0, tied = 0, behind = 0;
    int HP[3][3];
    memset(HP, 0, sizeof(HP));
    int HPTotal[3] = {0, 0, 0};
    for (int oc1 = 1; oc1 <= ONE_CARD_CODES - 1; oc1 ++)
        for (int oc2 = oc1 + 1; oc2 <= ONE_CARD_CODES; oc2 ++)
            if (F[oc1] + F[oc2] == 0)
            {
                F[oc1] ++; F[oc2] ++;
                int index = _inc_vars(ahead, tied, behind, _evaluateCards(pc1, pc2, oc1, oc2, tc1, tc2, tc3, tc4, tc5));
                if(!tc5)
                    HPTotal[index] += _computePotential(HP[index], F, pc1, pc2, oc1, oc2, tc1, tc2, tc3, tc4, tc5);
                F[oc1] --; F[oc2] --;
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
    if(tc5)
        return hs;
    else
    {
        if (HPTotal[BEHIND] + HPTotal[TIED])
            ppot = ((double)HP[BEHIND][AHEAD] + HP[BEHIND][TIED]/ 2.0 + HP[TIED][AHEAD] / 2.0) / (double)(HPTotal[BEHIND] + HPTotal[TIED]);
        if (HPTotal[AHEAD] + HPTotal[TIED])
            npot = ((double)HP[AHEAD][BEHIND] + HP[TIED][BEHIND] / 2.0 + HP[AHEAD][TIED] / 2.0) / (double)(HPTotal[AHEAD] + HPTotal[TIED]);
        double score = hs * (1.0 - npot) + (1.0 - hs) * ppot;
        //printf("hs: %lf ppot: %lf npot: %lf score: %lf\n", hs, ppot, npot, score);
        return score;
    }
}
int BasketManager::_computePotential(int HP[3], int F[ONE_CARD_CODES+ 3], int pc1, int pc2, int oc1,
                                      int oc2, int tc1, int tc2, int tc3, int tc4, int tc5)
{
    int total = 0;
    if (!tc1)
    {
        for (int t1 = 1; t1 <= ONE_CARD_CODES - 2; t1 ++)
            for (int t2 = t1 + 1; t2 <= ONE_CARD_CODES - 1; t2++)
                for (int t3 = t2 + 1; t3 <= ONE_CARD_CODES; t3++)
                    if (!(F[t1] + F[t2] + F[t3]))
                    {
                        HP[_evaluateCards(pc1, pc2, oc1, oc2, t1, t2, t3)] ++;
                        total ++;
                    }
    }
    else if (!tc4)
    {
        for (int t4 = 1; t4 <= ONE_CARD_CODES; t4++)
            if (!F[t4])
            {
                HP[_evaluateCards(pc1, pc2, oc1, oc2, tc1, tc2, tc3, t4)] ++;
                total ++;
            }

    }
    else if (!tc5)
    {
        for (int t5 = 1; t5 <= ONE_CARD_CODES; t5++)
        {
            if (!F[t5])
            {
                HP[_evaluateCards(pc1, pc2, oc1, oc2, tc1, tc2, tc3, tc4, t5)] ++;
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

int BasketManager::_computePotential2(int HP[3], int F[ONE_CARD_CODES+ 3], int pc1, int pc2, int oc1,
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
                            if (t1 != t4)
                            {
                                tc1 = _card_map(1, t1);
                                tc2 = _card_map(2, t2);
                                tc3 = _card_map(3, t3);
                                tc4 = _card_map(1, t4);
                                tc5 = _card_map(4, t5);
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
    //printf("stage: %d, ehs: %lf\n", stage, ehs);
    for (int b = 0; b < basket_sizes[stage]; b++)
        if (ehs <= THR[stage][b])
            return b;
    return basket_sizes[stage] - 1;
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


int BasketManager::getBasket(vector<int> cards)
{
    int n = cards.size();
    int i1 = CARD_CODES_MAP[0][_cardsCode(cards[0], cards[1])];
    if(n <= 2)
        return B1[index][i1];
    int i2 = CARD_CODES_MAP[1][_cardsCode2(cards[2], cards[3], cards[4])];
    if(n <= 5)
        return B2[index][i2][i1];
    int i3 = CARD_CODES_MAP[2][cards[5]];
    if(n <= 6)
        return B3[index][i3][i2][i1];
    int i4 = CARD_CODES_MAP[3][cards[6]];
    return B4[index][i4][i3][i2][i1];
}

int BasketManager::getIndex(int stage, vector<int> cards)
{
    return CARD_CODES_MAP[stage][cardsCode(cards)];
}

dist BasketManager::getOpponentBasketDist(int stage, vector<int> ind)
{
    vector<double> d;
    for (int i = 0; i < basket_sizes[stage]; i++)
        d.push_back(0.0);

    int* baskets_dist;
    if (stage == 0)
        baskets_dist = B1[index];
    else if (stage == 1)
        baskets_dist = B2[index][ind[1]];
    else if (stage == 2)
        baskets_dist = B3[index][ind[2]][ind[1]];
    else
        baskets_dist = B4[index][ind[3]][ind[2]][ind[1]];
    for (int i = 0; i < EHS_SIZE1; i++)
        d[baskets_dist[i]] += S1[i];

    double sum = 0.0;
    for (int i = 0; i < d.size(); i++)
        sum += d[i];
    dist res;
    if (sum > 0.0)
        for (int basket = 0; basket < d.size(); basket++)
            res.push_back(make_pair(basket, d[basket] / sum));
    return res;
}

dist BasketManager::getOpponentBasketDist(int stage, int player_basket, dist current_dist, vector<int> cards)
{
    // przerobic na iterowanie po i1 i jak wyzej
    vector<double> d;
    for (int i = 0; i < basket_sizes[stage]; i++)
        d.push_back(0.0);

    if (stage == 0)
    {
        dist pair_dist = BASKET_DISTRIBUTION[index][stage][0];
        for (int i = 0; i < pair_dist.size(); i++)
        {
            pair<int, int> bpair = decode_basket_pair(pair_dist[i].first);
            if (bpair.first == player_basket)
                d[bpair.second] = pair_dist[i].second;
        }
    }
    else
    {
        int cards_code = cardsCode(cards);
        for (int i = 0; i < current_dist.size(); i++)
        {
            int b0 = current_dist[i].first;
            double* trans;
            if (stage == 1)
                trans = FL[index][b0][cards_code];
            else
                trans = TR[index][stage-2][b0][cards_code];
            for (int b1 = 0; b1 < d.size(); b1 ++)
                d[b1] += trans[b1] * current_dist[i].second;
        }
    }

    double sum = 0.0;
    for (int i = 0; i < d.size(); i++)
        sum += d[i];
    dist res;
    if (sum > 0.0)
        for (int basket = 0; basket < d.size(); basket++)
            res.push_back(make_pair(basket, d[basket] / sum));
    return res;
}

int BasketManager::getNextBasket(int stage, int current, int cards_code)
{
    log(1, "stage: %d, current: %d, cards_code: %d\n", stage, current, cards_code);
    if (stage == 0) // pre flop
        return PF[index][cards_code];

    double *dist;
    if (stage == 1)
        dist = FL[index][current][cards_code];
    if (stage > 1)
        dist = TR[index][stage - 2][current][cards_code];

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

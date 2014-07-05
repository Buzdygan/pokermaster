#include<algorithm>
#include<vector>

#include "Utils.h"
#include "BasketManager.h"

const char* TRANSITIONS_FILENAME = "basket_transitions.stg";
const char* DISTRIBUTION_FILENAME = "basket_distribution.stg";

int CC[55][55];
int PF[TWO_CARD_CODES + 5]; // pre flop basket
double F[MAX_BASKETS_NUMBER][THREE_CARD_CODES + 10][MAX_BASKETS_NUMBER]; // flop basket transistions
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
            printf("DIST: %d sum: %d\n", DIST[bpair], sum);
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
                                        double prob = F[a0][cards_code][b0] * F[a1][cards_code][b1];
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
                fprintf(f, "%d %d %d %lf\n", b1, cc, b2, F[b1][cc][b2]);

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
            F[b1][cc][b2] = val;
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

/*
 * Plan:
 * Pierwsza faza - liczymy prawdopodobieństwa dla dwóch kart, przeglądamy wszystkie możliwe kombinacje dwójek
 * naszych i przeciwnika i trójek na flopie. Dla każdej dwójki liczymy prawdopodobieństwo wygrania na podstawie 5 kart
 * Przydzielamy koszyki.
 * Potem będzi
 */

void BasketManager::_computeTransitions()
{

    _computeFirstBasket();

    double val = 1.0 / basket_sizes[1];
    for (int b1 = 0; b1 < basket_sizes[0]; b1++)
        for (int cc = 0; cc < THREE_CARD_CODES; cc++)
            for (int b2 = 0; b2 < basket_sizes[1]; b2++)
                F[b1][cc][b2] = val;

    for (int i = 0; i < 2; i++)
    {
        double val = 1.0 / basket_sizes[2 + i];
        for (int b1 = 0; b1 < basket_sizes[1 + i]; b1++)
            for (int cc = 0; cc < ONE_CARD_CODES; cc++)
                for (int b2 = 0; b2 < basket_sizes[2 + i]; b2++)
                    TR[i][b1][cc][b2] = val;
    }
}

void BasketManager::_computeFirstBasket()
{
    printf("Computing first basket\n");
    for (int p1 = 1; p1 <= 4 * FIGS; p1 += 4)
        for (int p2 = p1; p2 <= 4 * FIGS; p2 += 4)
        {
            if (p1 == p2) // the same figure/number
            {
                int basket = _computeFirstBasket(p1, p2 + 1);
                for (int c0 = p1; c0 < p1 + 4; c0 ++)
                    for (int c1 = c0 + 1; c1 < p1 + 4; c1 ++)
                        PF[_cardsCode(c0, c1)] = basket;
            }
            else // different figure/number
            {
                // for the same coloured cards
                int basket = _computeFirstBasket(p1, p2);
                for (int i = 0; i < 4; i++)
                    PF[_cardsCode(p1 + i, p2 + i)] = basket;

                // for the differently coloured cards
                basket = _computeFirstBasket(p1, p2 + 1);
                for (int c0 = p1; c0 < p1 + 4; c0 ++)
                    for (int c1 = p2; c1 < p2 + 4; c1 ++)
                        if ((c0 % 4) != (c1 % 4))
                            PF[_cardsCode(c0, c1)] = basket;
            }
        }
}

int BasketManager::_computeFirstBasket(int c1, int c2)
{
    int F[FIGS + 2];
    memset(F, 0, sizeof(F));
    F[(c1 - 1) / 4] ++;
    F[(c2 - 1) / 4] ++;
    int wins = 0;
    int all = 0;
    for (int o1 = 0; o1 < FIGS; o1 ++)
        for (int o2 = o1; o2 < FIGS; o2 ++)
        {
            F[o1] ++;
            F[o2] ++;
            int cnt = 0;
            for (int t1 = 0; t1 < FIGS; t1 ++)
                if (F[t1] <= 4)
                {
                    F[t1] ++;
                    for (int t2 = t1; t2 < FIGS; t2 ++)
                        if(F[t2] <= 4)
                        {
                            F[t2] ++;
                            for (int t3 = t2; t3 < FIGS; t3++)
                                if (F[t3] <= 4)
                                {
                                    int co1 = 1 + o1 * 4, co2 = 1 + o2 * 4, ct1 = 1 + t1 * 4, ct2 = 1 + t2 * 4, ct3 = 1 + t3 * 4;

                                    cnt += _evaluateCards(c1, c2, co1, co2 + 1, ct1, ct2 + 1, ct3 + 2);
                                    if (t1 != t2)
                                    {
                                        cnt += _evaluateCards(c1, c2, co1, co2 + 1, ct1, ct2, ct3 + 1);
                                        if (t2 != t3)
                                        {
                                            cnt += _evaluateCards(c1, c2, co1, co2 + 1, ct1, ct2, ct3);
                                        }
                                    }
                                    if (t2 != t3)
                                        cnt += _evaluateCards(c1, c2, co1, co2 + 1, ct1 + 1, ct2, ct3);
                                    if (t1 != t3)
                                        cnt += _evaluateCards(c1, c2, co1, co2 + 1, ct1, ct2 + 1, ct3);

                                    if (o1 != o2)
                                    {
                                        cnt += _evaluateCards(c1, c2, co1, co2, ct1, ct2 + 1, ct3 + 2);
                                        if (t1 != t2)
                                        {
                                            cnt += _evaluateCards(c1, c2, co1, co2, ct1, ct2, ct3 + 1);
                                            if (t2 != t3)
                                            {
                                                cnt += _evaluateCards(c1, c2, co1, co2, ct1, ct2, ct3);
                                            }
                                        }
                                        if (t2 != t3)
                                            cnt += _evaluateCards(c1, c2, co1, co2, ct1 + 1, ct2, ct3);
                                        if (t1 != t3)
                                            cnt += _evaluateCards(c1, c2, co1, co2, ct1, ct2 + 1, ct3);
                                    }
                                }
                            F[t2] --;
                        }
                    F[t1] --;
                }
            F[o1] --;
            F[o2] --;
            //printf("cnt: %d\n", cnt);
            if (cnt > 0)
                wins ++;
            all ++;
        }
    printf("wins: %d, all: %d\n", wins, all);
    return _determineBasket(0, double(wins) / all);
}

int BasketManager::_evaluateCards(int p1, int p2, int o1, int o2, int t1, int t2, int t3, int t4, int t5)
{
    int pscore = evaluator -> evaluateHand(p1, p2, t1, t2, t3, t4, t5);
    int oppscore = evaluator -> evaluateHand(o1, o2, t1, t2, t3, t4, t5);
    if (pscore > oppscore)
        return 1;
    if (pscore == oppscore)
        return 0;
    if (pscore < oppscore)
        return -1;
}


/* returns basket based on the stage and probability of win */
int BasketManager::_determineBasket(int stage, double win_prob)
{
    // TODO
    return 0;
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
        dist = F[current][cards_code];
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

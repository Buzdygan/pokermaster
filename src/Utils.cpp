#include<algorithm>
#include<vector>

#include "Utils.h"

const char* TRANSITIONS_FILENAME = "transitions.txt";

int HR[32487834];
int CC[55][55];
int PF[TWO_CARD_CODES + 5]; // pre flop basket
double F[MAX_BASKETS_NUMBER][THREE_CARD_CODES + 10][MAX_BASKETS_NUMBER]; // flop basket transistions
double TR[2][MAX_BASKETS_NUMBER][ONE_CARD_CODES + 5][MAX_BASKETS_NUMBER]; // turn and river basket transitions

int other(int player_number)
{
    return (player_number + 1) & 1;
}

void printCard(int card)
{
    char F[13] = {'2', '3', '4', '5', '6', '7', '8', '9', 'T', 'W', 'D', 'K', 'A'};
    char C[4] = {'c', 'd', 'h', 's'};
    card --;
    printf("%c%c ", F[card / 4], C[card % 4]);
}

void printHand(vector<int> cards)
{
    for (int i = 0; i < cards.size(); i++)
    {
        printCard(cards[i]);
        printf(" ");
    }
}

HandEvaluator::HandEvaluator(int bs[4])
{
	memset(HR, 0, sizeof(HR));
	FILE * fin = fopen("../data/HandRanks.dat", "rb");
	size_t bytesread = fread(HR, sizeof(HR), 1, fin);	// get the HandRank Array
	fclose(fin);
    basket_sizes = new int[4];
    for (int b = 0; b < 4; b++)
        basket_sizes[b] = bs[b];
    _computeCC();
}

HandEvaluator::HandEvaluator()
{
	memset(HR, 0, sizeof(HR));
	FILE * fin = fopen("../data/HandRanks.dat", "rb");
	size_t bytesread = fread(HR, sizeof(HR), 1, fin);	// get the HandRank Array
	fclose(fin);
    basket_sizes = new int[4];
    for (int b = 0; b < 4; b++)
        basket_sizes[b] = MAX_BASKETS_NUMBER;
    _computeCC();
    if (!loadTransitions())
    {
        computeTransitions();
        saveTransitions();
    }
}

void HandEvaluator::saveTransitions()
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

bool HandEvaluator::loadTransitions()
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

void HandEvaluator::computeTransitions()
{

}

int HandEvaluator::cardsCode(vector<int> cards)
{
    int n = cards.size();
    if (n == 1)
        return cards[0];
    sort(cards.begin(), cards.end());
    if (n == 2)
        return cards[0] + cards[1] * 53;
    return CC[cards[0]][cards[1]] + cards[2] - cards[1] - 1;
}

int HandEvaluator::getNextBasket(int stage, int current, int cards_code)
{
    if (stage == 0) // pre flop
        return PF[cards_code];
    double* baskets_distribution;
    if (stage == 1)
        baskets_distribution = F[current][cards_code];
    if (stage > 1)
        baskets_distribution = TR[stage - 2][current][cards_code];
    int n = basket_sizes[stage]; // number of baskets in this stage

    // now choose the basket according to the distribution
    double r = (double) rand() / RAND_MAX;
    double sum = 0.0;
    for (int i = 0; i < n; i ++)
    {
        sum += baskets_distribution[i];
        if (r < sum)
            return i;
    }
    return 0;
}

int HandEvaluator::evaluateHand(vector<int> cards)
{
    int ind = 0;
    int p = HR[53 + cards[ind++]];
    p = HR[p + cards[ind++]];
    p = HR[p + cards[ind++]];
    p = HR[p + cards[ind++]];
    p = HR[p + cards[ind++]];
    p = HR[p + cards[ind++]];
    return HR[p + cards[ind++]];
}

void HandEvaluator::test()
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

void HandEvaluator::_computeCC()
{
    int cnt = 0;
    for (int i = 1; i <= 52; i++)
        for (int j = i+1; j <= 52; j++)
        {
            CC[i][j] = cnt;
            cnt += 52 - j;
        }
}

#include <cstdlib>
#include <ctime>
#include <vector>
#include <cstdio>

#include "GameAbstraction.h"
#include "HoldemPoker.h"
#include "HoldemPokerModAbstraction.h"
#include "Utils.h"
#include "Player.h"
#include "DummyPlayer.h"
#include "CfrPlayer.h"
#include "CfrModPlayer.h"
#include "CfrAbstractionPlayer.h"
#include "HumanPlayer.h"

const int DEFAULT_ROUNDS_NUMBER = 1;
const int DEFAULT_STRATEGY_REPETITIONS = 10;

bool EHS_POTENTIAL = true;
char EHS_STR [7] = "pot";

int get_random_action(dist distribution)
{
    double r = (double) rand() / RAND_MAX;
    for (int i = 0; i < 100; i ++)
        r = (double) rand() / RAND_MAX;

    double sum = 0.0;
    for (int i = 0; i < distribution.size(); i ++)
    {
        sum += distribution[i].second;
        if (r < sum)
            return distribution[i].first;
    }
    return distribution[0].first;
}

void testBaskets(BasketManager* mng)
{
    //23 16 44 22 17
    while(1)
    {
        int c1, c2, c3;
        vector<int> cards;
        printf("R1\n");
        scanf("%d %d", &c1, &c2);
        cards.push_back(c1);
        cards.push_back(c2);
        printHand(cards);
        printf(" Basket: %d\n", mng -> getBasket(cards));

        printf("R2\n");
        scanf("%d %d %d", &c1, &c2, &c3);
        cards.push_back(c1);
        cards.push_back(c2);
        cards.push_back(c3);
        printHand(cards);
        printf(" Basket: %d\n", mng -> getBasket(cards));

        printf("R3\n");
        scanf("%d", &c1);
        cards.push_back(c1);
        printHand(cards);
        printf(" Basket: %d\n", mng -> getBasket(cards));

        printf("R4\n");
        scanf("%d", &c1);
        cards.push_back(c1);
        printHand(cards);
        printf(" Basket: %d\n", mng -> getBasket(cards));
    }

}

int main(int argc, char* argv[])
{
    srand(time(NULL));
    int rounds_number = DEFAULT_ROUNDS_NUMBER;
    int strategy_repetitions = DEFAULT_STRATEGY_REPETITIONS;
    int strategy_repetitions2 = DEFAULT_STRATEGY_REPETITIONS;

    if (argc >= 2)
        rounds_number = atoi(argv[1]);

    if (argc >= 3)
        strategy_repetitions = atoi(argv[2]);

    if (argc >= 4)
        strategy_repetitions2 = atoi(argv[3]);

    char strategy_filename [100];
    char strategy_filename2 [100];

    Player* players[2];
    int score[2];
    score[0] = 0;
    score[1] = 0;
    int basket_sizes[4] = {12,10,10,10};
    HandEvaluator evaluator;

    BasketManager mng(0, basket_sizes, &evaluator, EHS_POTENTIAL, EHS_STR);
    testBaskets(&mng);
    return 0;
}

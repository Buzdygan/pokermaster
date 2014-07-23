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
#include "ModCfr.h"
#include "Cfr.h"
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

void test_cards(HandEvaluator *evaluator)
{
    while(1)
    {
        printf("Podaj karty\n");
        int c = 1;
        vector<int> cards;
        while (c >= 0)
        {
            scanf("%d", &c);
            if (c < 0)
                break;
            cards.push_back(c);
        }
        printHand(cards);
        int score = evaluator -> evaluateHand(cards);
        printf("score: %d, cat: %d\n", score, score >> 12);
    }
}

void test_baskets(BasketManager* mng)
{
    for (int i = 1; i <= 51; i++)
        for (int j = i+1; j <= 52; j++)
        {
            vector<int> cards;
            cards.push_back(i);
            cards.push_back(j);
            printHand(cards);
            printf("basket: %d\n", mng -> getBasket(cards));
        }

    while(1)
    {
        printf("Podaj karty\n");
        int c = 1;
        vector<int> cards;
        while (c >= 1)
        {
            scanf("%d", &c);
            if (c <= 0)
                break;
            cards.push_back(c);
        }
        printHand(cards);
        printf("basket: %d\n", mng -> getBasket(cards));
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
    //int basket_sizes[4] = {3,3,3,3};
    sprintf(strategy_filename, "cfr.mod.strategy%s-%d-%d-%d-%d-%d.stg", EHS_STR,
                                                                       basket_sizes[0],
                                                                       basket_sizes[1],
                                                                       basket_sizes[2],
                                                                       basket_sizes[3],
                                                                       strategy_repetitions);

    int basket_sizes2[4] = {5,4,4,3};

    sprintf(strategy_filename2, "cfr.strategy%s-%d-%d-%d-%d-%d.stg", EHS_STR,
                                                                       basket_sizes2[0],
                                                                       basket_sizes2[1],
                                                                       basket_sizes2[2],
                                                                       basket_sizes2[3],
                                                                       strategy_repetitions2);


    HandEvaluator evaluator;

    BasketManager mng(0, basket_sizes, &evaluator, EHS_POTENTIAL, EHS_STR);
    BasketManager mng2(1, basket_sizes2, &evaluator, EHS_POTENTIAL, EHS_STR);
    ModCfr *cfr_mod_strategy = new ModCfr(new HoldemPokerModAbstraction(&mng), strategy_repetitions, strategy_filename);
    Cfr *cfr_strategy = new Cfr(new HoldemPokerAbstraction(&mng2), strategy_repetitions2, strategy_filename2, false);

    for (int r = 0; r < rounds_number; r++)
    {
        // new round
        GameAbstraction* game = new HoldemPoker(&evaluator);
        //GameAbstraction* game = new HoldemPokerAbstraction(&mng);
        printf("RUNDAAAAAAAAAAAAAAAAAAAAAAAAAA: %d\n", r);



        /*
        players[r & 1] = new CfrPlayer(r & 1,
                                       cfr_strategy,
                                       new HoldemPokerAbstraction(&mng));
                                       */
        /*
        players[r & 1] = new CfrAbstractionPlayer(r & 1,
                                       cfr_strategy,
                                       new HoldemPokerAbstraction(&mng));
                                       */

        /*
        players[r & 1] = new CfrModPlayer(r & 1,
                                       cfr_mod_strategy,
                                       new HoldemPokerModAbstraction(&mng));
                                       */
        /*
        players[r & 1] = new HumanPlayer(r & 1);
        */

        players[r & 1] = new DummyPlayer();

        players[(r + 1) & 1] = new CfrPlayer((r + 1) & 1,
                                       cfr_strategy,
                                       new HoldemPokerAbstraction(&mng2));


        //players[(r + 1) & 1] = new DummyPlayer();

        vector<int> players_cards[2];

        printf("CFR MOD: %d, CFR: %d\n", r & 1, (r+1) & 1);
        while (!game -> isFinal())
        {
            int pnum = game -> getPlayerId();
            if (pnum == RANDOM_PLAYER_NR)
            {
                dist distr = game -> getActionDistribution();
                int action_id = get_random_action(distr);
                int seeing_player = game -> randomActionPlayer();
                game -> makeAction(action_id);
                if (seeing_player == ALL_PLAYERS)
                {
                    printCard(action_id);
                    log(0, " dealt to player %d\n", seeing_player);
                    for (int p = 0; p < 2; p++)
                    {
                        players[p] -> annotateRandomAction(action_id);
                        players_cards[p].push_back(action_id);
                    }
                }
                else
                {
                    printCard(action_id);
                    log(0, " dealt to player %d\n", seeing_player);
                    players[seeing_player] -> annotateRandomAction(action_id);
                    // the player shouldn't see the real action but it's needed
                    // to keep the other player's game tree
                    players_cards[seeing_player].push_back(action_id);
                }
            }
            else
            {
                int action_id = players[pnum] -> getAction(game -> getActionIds());
                log(0, "Player %d plays %d\n", pnum, action_id);
                game -> makeAction(action_id);
                players[other(pnum)] -> annotateOpponentAction(action_id);
                players[pnum] -> annotatePlayerAction(action_id);
            }
        }
        utility round_result = game -> getUtility();
        players[0] -> endRound(round_result.first);
        players[1] -> endRound(round_result.second);
        score[r & 1] += round_result.first;
        score[(r + 1) & 1] += round_result.second;
        printf("End of round %d, results: %1.f:%1.f, current score: %d:%d\n", r, round_result.first,
                round_result.second, score[0], score[1]);
        for (int j = 0; j < 2; j++)
        {
            printf("Player %d cards: ", j);
            for (int c = 0; c < players_cards[j].size(); c++)
                printCard(players_cards[j][c]);
            printf("\n");
        }
        delete game;
    }
    return 0;
}


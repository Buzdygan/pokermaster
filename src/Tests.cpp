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
    int basket_sizes[4] = {5,5,5,5};
    sprintf(strategy_filename, "cfr.mod.strategy%s-%d-%d-%d-%d-%d.stg", EHS_STR,
                                                                       basket_sizes[0],
                                                                       basket_sizes[1],
                                                                       basket_sizes[2],
                                                                       basket_sizes[3],
                                                                       strategy_repetitions);
    HandEvaluator evaluator;

    BasketManager mng(0, basket_sizes, &evaluator, EHS_POTENTIAL, EHS_STR);
    Cfr *cfr_mod_strategy = new Cfr(new HoldemPokerModAbstraction(&mng), strategy_repetitions, strategy_filename, true);

    for (int r = 0; r < rounds_number; r++)
    {
        // new round
        GameAbstraction* game = new HoldemPokerModAbstraction(&mng);
        printf("RUNDAAAAAAAAAAAAAAAAAAAAAAAAAA: %d\n", r);


        players[r & 1] = new CfrAbstractionPlayer(r & 1,
                                       cfr_mod_strategy,
                                       new HoldemPokerModAbstraction(&mng));

        players[(r + 1) & 1] = new DummyPlayer();

        vector<int> players_cards[2];

        printf("CFR: %d, DUMMY: %d\n", r & 1, (r+1) & 1);
        while (!game -> isFinal())
        {
            int pnum = game -> getPlayerId();
            if (pnum == RANDOM_PLAYER_NR)
            {
                dist distr = game -> getActionDistribution();
                int action_id = get_random_action(distr);
                game -> makeAction(action_id);
                for (int p = 0; p < 2; p++)
                    players[p] -> annotateRandomAction(action_id);
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
        delete game;
    }
    return 0;
}

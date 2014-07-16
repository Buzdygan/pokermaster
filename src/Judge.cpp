#include <cstdlib>
#include <ctime>
#include <vector>

#include "GameAbstraction.h"
#include "HoldemPoker.h"
#include "Utils.h"
#include "Player.h"
#include "DummyPlayer.h"
#include "CfrPlayer.h"
#include "HumanPlayer.h"

const int DEFAULT_ROUNDS_NUMBER = 1;
const int DEFAULT_STRATEGY_REPETITIONS = 10;


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

int main(int argc, char* argv[])
{
    srand(time(NULL));
    int rounds_number = DEFAULT_ROUNDS_NUMBER;
    int strategy_repetitions = DEFAULT_STRATEGY_REPETITIONS;

    if (argc >= 2)
        rounds_number = atoi(argv[1]);

    if (argc >= 3)
        strategy_repetitions = atoi(argv[2]);

    char strategy_filename [100];
    sprintf(strategy_filename, "cfr.strategy%d.stg", strategy_repetitions);

    Player* players[2];
    int score[2];
    score[0] = 0;
    score[1] = 0;
    int basket_sizes[4] = {6,5,5,5};
    HandEvaluator evaluator;
    //test_cards(&evaluator);
    //TODO invalidate when the basket sizes change
    BasketManager mng(basket_sizes, &evaluator);
    Cfr *cfr_strategy = new Cfr(new HoldemPokerAbstraction(&mng), strategy_repetitions, strategy_filename);
    //Cfr *cfr_strategy2 = new Cfr(new SimplePoker(), 100, "cfr.strategy100");

    for (int r = 0; r < rounds_number; r++)
    {
        // new round
        GameAbstraction* game = new HoldemPoker(&evaluator);
        int game_type = 2;
        if (game_type == 0)
        {
            players[r & 1] = new DummyPlayer();
            players[(r + 1) & 1] = new DummyPlayer();
        }
        if (game_type == 1)
        {
            players[r & 1] = new HumanPlayer(r & 1);
            players[(r + 1) & 1] = new HumanPlayer((r + 1) & 1);
        }
        if (game_type == 2)
        {
            players[r & 1] = new CfrPlayer(r & 1,
                                           cfr_strategy,
                                           new HoldemPokerAbstraction(&mng));
            players[(r + 1) & 1] = new DummyPlayer();
        }
        vector<int> players_cards[2];

        printf("CFR: %d, Dummy: %d\n", r & 1, (r+1) & 1);
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
                log(0, "Player %d bets %d\n", pnum, action_id);
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


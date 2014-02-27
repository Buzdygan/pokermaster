#include <cstdlib>
#include <ctime>
#include <vector>

#include "GameAbstraction.h"
#include "SimplePoker.h"
#include "Utils.h"
#include "Player.h"
#include "DummyPlayer.h"
#include "CfrPlayer.h"
#include "HumanPlayer.h"

int get_random_action(dist distribution)
{
    double r = (double) rand() / RAND_MAX;
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
    srand(time(0));
    int rounds_number = DEFAULT_ROUNDS_NUMBER;

    if (argc == 2)
        rounds_number = atoi(argv[1]);

    Player* players[2];
    int score[2];
    players[0] = new DummyPlayer();
    players[1] = new CfrPlayer(new Cfr(new SimplePoker()));

    for (int r = 0; r < rounds_number; r++)
    {
        /* new round */
        GameAbstraction* game = new SimplePoker();
        for (int p = 0; p < 2; p++)
            players[p] -> startNewRound();

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
                    for (int p = 0; p < 2; p++)
                        players[p] -> annotateRandomAction(action_id);
                }
                else
                    players[seeing_player] -> annotateRandomAction(action_id);
                printf("Card %d dealt to player %d\n", action_id, seeing_player);
            }
            else
            {
                int action_id = players[pnum] -> getAction(game -> getInformationSetId(),
                                                           game -> getActionIds());
                printf("Player %d bets %d\n", pnum, action_id);
                game -> makeAction(action_id);
                players[other(pnum)] -> annotateOpponentAction(action_id);
            }
        }
        utility round_result = game -> getUtility();
        players[0] -> endRound(round_result.first);
        players[1] -> endRound(round_result.second);
        score[0] += round_result.first;
        score[1] += round_result.second;
        printf("End of round %d, results: %1.f:%1.f, current score: %d:%d\n", r, round_result.first,
                round_result.second, score[0], score[1]);
        delete game;
    }
    return 0;
}


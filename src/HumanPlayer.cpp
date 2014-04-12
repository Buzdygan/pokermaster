#include <utility>
#include <cstdio>
#include <cmath>
#include <vector>
#include "Utils.h"
#include "HumanPlayer.h"

using namespace std;

HumanPlayer::HumanPlayer()
{
}

/* starts new round */
void HumanPlayer::startNewRound()
{
    printf("HUMAN: Starting new round\n");
}
/* Annotates random action */
void HumanPlayer::annotateRandomAction(int action_id)
{
    printf("HUMAN: CARD ");
    printCard(action_id);
    printf("dealt\n", action_id);

}
/* annotates opponent's action */
void HumanPlayer::annotateOpponentAction(int action_id)
{
    printf("HUMAN: Opponent bets %d\n", action_id);
}

/* Get player's action */
int HumanPlayer::getAction(int information_set_id, vector<int> available_actions)
{
    printf("HUMAN: Select action from: ");
    for (int i = 0; i < available_actions.size(); i++)
        printf("%d ", available_actions[i]);
    printf("\n");
    int action_id;
    scanf("%d", &action_id);
    return action_id;
}
/* gives info on who won the round with what stake */
void HumanPlayer::endRound(double cash_change)
{
    printf("HUMAN: Your score: %.1lf\n", cash_change);
}


#include <utility>
#include <cstdio>
#include <cmath>
#include <vector>
#include "Utils.h"
#include "HumanPlayer.h"

using namespace std;

HumanPlayer::HumanPlayer(int p)
{
    pnum = p;
}

/* Annotates random action */
void HumanPlayer::annotateRandomAction(int action_id)
{
    printf("HUMAN %d: CARD ", pnum);
    printCard(action_id);
    printf("dealt\n", action_id);

}

/* annotates this player's action */
void HumanPlayer::annotatePlayerAction(int action_id)
{
    printf("HUMAN %d: You play %d\n", pnum, action_id);
}

/* annotates opponent's action */
void HumanPlayer::annotateOpponentAction(int action_id)
{
    printf("HUMAN %d: Opponent plays %d\n", pnum, action_id);
}

/* Get player's action */
int HumanPlayer::getAction(vector<int> available_actions)
{
    printf("HUMAN %d: Select action from: ", pnum);
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
    printf("HUMAN %d: Your score: %.1lf\n", pnum, cash_change);
}


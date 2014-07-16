#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <vector>
#include <set>

#include "GameAbstraction.h"
#include "SimplePoker.h"
#include "Utils.h"
#include "Player.h"
#include "DummyPlayer.h"
#include "HumanPlayer.h"

set<int> information_sets[2];

void printDepth(int depth)
{
    for (int i = 0; i < depth; i++)
        printf("  ");
}

int traverse(GameAbstraction* game, int depth, bool debug=false)
{
    int pid = game -> getPlayerId();
    if (debug)
    {
        printDepth(depth);
        if (pid == RANDOM_PLAYER_NR)
            printf("Random Player\n");
        else
            printf("Player: %d Game information set id: %d\n", pid, game -> getInformationSetId());
    }
    if (pid != RANDOM_PLAYER_NR)
    {
        int is_id = game -> getInformationSetId();
        if (information_sets[pid].count(is_id) > 0)
        {
            printf("Error, information set id: %d appeared twice for %d\n", is_id, pid);
            return 1;
        }
        information_sets[pid].insert(is_id);
    }
    if (game -> isFinal())
    {
        printDepth(depth);
        printf("Reached leaf\n");
        return 0;
    }
    vector<int> action_ids = game -> getActionIds();
    for (int i = 0; i < action_ids.size(); i++)
    {
        game -> makeAction(action_ids[i]);
        if (debug)
        {
            printDepth(depth);
            printf("Player %d makes action %d\n", pid, action_ids[i]);
        }
        int err = traverse(game, depth + 1, debug);
        game -> unmakeAction(action_ids[i]);
        if (err != 0)
            return err;
    }
    return 0;
}

int test_information_sets_simple_poker()
{
    printf("Testing information set id uniqueness in SimplePoker\n");
    GameAbstraction* game = new SimplePoker();
    return traverse(game, 0, true);
}


int main(int argc, char* argv[])
{
    int failed = 0;
    //failed += test_information_sets_simple_poker();
    printf("Failed number: %d\n", failed);
    return 0;
}

#include <cstdlib>
#include <ctime>
#include <vector>

#include "Poker.h"
#include "Utils.h"
#include "Player.h"
#include "DummyPlayer.h"
#include "HumanPlayer.h"

int main(int argc, char* argv[])
{
    srand(time(0));

    int start_cash = DEFAULT_INITIAL_CASH;
    int rounds_number = DEFAULT_ROUNDS_NUMBER;

    if (argc == 2)
        start_cash = atoi(argv[1]);

    if (argc == 3)
        rounds_number = atoi(argv[2]);

    Player* players[2];
    players[0] = new DummyPlayer(0, start_cash);
    players[1] = new DummyPlayer(1, start_cash);
    Poker game(players[0], players[1], start_cash);
    game.play(rounds_number);
    return 0;
}


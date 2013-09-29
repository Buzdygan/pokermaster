#include <cstdlib>
#include <ctime>

#include "Poker.h"
#include "Utils.h"
#include "Player.h"
#include "DummyPlayer.h"

int main(int argc, char* argv[])
{
    srand(time(0));

    int p1_cash = DEFAULT_INITIAL_CASH;
    int p2_cash = DEFAULT_INITIAL_CASH;

    if (argc == 2)
        p1_cash = p2_cash = atoi(argv[1]);
    if (argc == 3)
    {
        p1_cash = atoi(argv[1]);
        p2_cash = atoi(argv[2]);
    }
    Player* players[2];
    players[0] = new DummyPlayer(0, p1_cash);
    players[1] = new DummyPlayer(1, p2_cash);
    Poker game(players[0], players[1]);
    game.play(p1_cash, p2_cash);

    return 0;
}

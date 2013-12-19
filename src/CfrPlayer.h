#ifndef CFR_PLAYER_H
#define CFR_PLAYER_H

#include <vector>
#include "Player.h"
using namespace std;

class CfrPokerHelper
{
    public:
        CfrPokerHelper(int cash[2], int first_player);
        int getInformationSetId();
        double getActionProbability(int actionId);
        vector<int> getActionIds();

        int getBetByActionId(int actionId);


};

class CfrPlayer : public Player
{
    public:
        CfrPlayer(int p_number, int p_cash, int buckets_number, int max_stake);
        /* starts new round with two dealt cards */
        void startNewRound(pair<int, int> cards);
        /* starts new phase with new cards */
        void startNewPhase(vector<int> cards);
        /* learns opponent cards */
        void showOpponentCards(pair<int, int> cards);
        /* gets bet in this phase, given opponent's bet. Bet -1 signifies start of the phase */
        int getBet(int opponent_bet);
        /* gives info on who won the round with what stake */
        void announceRoundWinner(int winner, int stake);
    private:
        int number, cash;
        void initFromFile(char* filename);
        void saveToFile(char* filename);
        void init();
};

#endif

#include "Cfr.h"

using namespace std;

const char* Cfr::DEFAULT_FILE = "cfr.strategy";
const char Cfr::FILE_DELIM = '\n';

Cfr::Cfr(CfrGameAbstraction* gm)
{
    srand(time(0));
    if (!loadFromFile(DEFAULT_FILE))
    {
        computeVanillaCfr();
        saveToFile(DEFAULT_FILE);
    }
}

void Cfr::computeVanillaCfr()
{
    initialized = false;
    int number_of_iterations = 1;
    for (int i = 0; i < number_of_iterations; i++)
    {
        double probs [3] = {1.0, 1.0, 1.0};
        walkTree(probs);
        initialized = true;
    }
}

utility Cfr::walkTree(double probs[3])
{
    if (game -> isFinal())
        return game -> getUtility();
    int p = game -> getPlayerId();
    utility final_util = make_pair(0.0, 0.0);

    /* If it is a turn of a chance player */
    if (p == RANDOM_PLAYER_NR)
    {
        dist action_distr = game -> getActionDistribution();
        for (dist_it iter = action_distr.begin(); iter != action_distr.end(); iter++)
        {
            double backup_prob = probs[RANDOM_PLAYER_NR];
            probs[RANDOM_PLAYER_NR] *= iter -> second;

            game -> makeAction(iter -> first);
            utility res_util = walkTree(probs);
            game -> unmakeAction(iter -> first);

            probs[RANDOM_PLAYER_NR] = backup_prob;
            final_util.first += res_util.first * iter -> second;
            final_util.second += res_util.second * iter -> second;
        }
    }
    else
    {
        int is_id = game -> getInformationSetId();
        vector<int> action_ids = game -> getActionIds();
        if (!initialized)
        {
            for (vi_it a_id = action_ids.begin(); a_id != action_ids.end(); a_id ++)
            {
                pair<int, int> decision_id = make_pair(is_id, *a_id);
                strategy[decision_id] = 1.0 / action_ids.size();
                R[decision_id] = 0.0;
                S[decision_id] = 0.0;
            }
        }

        double prob_mult = probs[2];
        for (int i = 0; i < 2; i ++)
            if (p != i)
                prob_mult *= probs[i];

        for (vi_it a_id = action_ids.begin(); a_id != action_ids.end(); a_id ++)
        {
            pair<int, int> decision_id = make_pair(is_id, *a_id);
            double action_prob = strategy[decision_id];
            double backup_prob = probs[RANDOM_PLAYER_NR];
            probs[p] *= action_prob;

            game -> makeAction(*a_id);
            utility temp_util = walkTree(probs);
            double res_util [2] = {temp_util.first, temp_util.second};
            game -> unmakeAction(*a_id);

            probs[RANDOM_PLAYER_NR] = backup_prob;


            R[decision_id] -= res_util[p] * prob_mult;
            final_util.first += temp_util.first * action_prob;
            final_util.second += temp_util.second * action_prob;
            S[decision_id] += probs[p] * action_prob;
        }

        double add = prob_mult;
        if (p == 0)
            add *= final_util.first;
        if (p == 1)
            add *= final_util.second;

        for (vi_it a_id = action_ids.begin(); a_id != action_ids.end(); a_id ++)
        {
            pair<int, int> decision_id = make_pair(is_id, *a_id);
            R[decision_id] += add;
        }
    }
    return final_util;
}


int Cfr::getActionId()
{
    int is_id = game -> getInformationSetId();
    vector<int> action_ids = game -> getActionIds();
    double random_double = ((double) rand() / (RAND_MAX));
    double prob_sum = 0;
    for (int i = 0; i < action_ids.size(); i++)
    {
        int action_id = action_ids[i];
        prob_sum += strategy[make_pair(is_id, action_id)];
        if (random_double < prob_sum)
            return action_id;
    }
    return -1;
}

void Cfr::makeAction(int action_id)
{
    game -> makeAction(action_id);
}


bool Cfr::loadFromFile(const char* filename)
{
    FILE *f = fopen(filename, "r");
    int strategy_size;
    fscanf(f, "%d\n", &strategy_size);
    for (int i = 0 ; i < strategy_size; i++)
    {
        int is_id, action_id;
        double prob;
        // TODO(fix the delimiter or just forget about it)
        fscanf(f, "%d %d %lf\n", &is_id, &action_id, &prob);
        strategy[make_pair(is_id, action_id)] = prob;
    }
    fclose(f);
}
void Cfr::saveToFile(const char* filename)
{
    FILE *f = fopen(filename, "w");
    fprintf(f, "%d\n", (int)strategy.size());
    for (stg_it iter = strategy.begin(); iter != strategy.end(); iter++)
    {
        int is_id = iter -> first.first;
        int action_id = iter -> first.second;
        double prob = iter -> second;
        fprintf(f, "%d %d %lf%c", is_id, action_id, prob, FILE_DELIM);
    }
    fclose(f);
}

#include "Cfr.h"
#include "GameAbstraction.h"

using namespace std;

const char* Cfr::DEFAULT_FILE = "cfr.strategy";
const char Cfr::FILE_DELIM = '\n';
const int Cfr::ITERATIONS = 2000;

Cfr::Cfr(GameAbstraction* gm, int iterations, const char* strategy_file)
{
    printf("Init Cfr, iterations: %d\n", iterations);
    game = gm;
    if (!loadFromFile(strategy_file))
    {
        srand(time(0));
        computeVanillaCfr(iterations);
        saveToFile(strategy_file);
    }
}

void Cfr::computeVanillaCfr(int iterations)
{
    printf("Computing CFR strategy\n");
    initialized = false;
    double sum = 0.0;
    for (int i = 0; i < iterations; i++)
    {
        printf("Iteration %d\n", i);
        double probs [3] = {1.0, 1.0, 1.0};
        walkTree(probs);
        initialized = true;
        double it_err = recomputeStrategy(R) / (i + 1);
        sum += it_err;
        printf("It err: %0.5f Err: %0.5f\n", it_err, sum / (i+1));
    }
    recomputeStrategy(S);
}

utility Cfr::walkTree(double probs[3])
{
    if (game -> isFinal())
    {
        return game -> getUtility();
    }
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
                if (!strategy.count(decision_id))
                    strategy[decision_id] = 1.0 / action_ids.size();
                if (!R.count(decision_id))
                    R[decision_id] = 0.0;
                if (!S.count(decision_id))
                    S[decision_id] = 0.0;
            }
        }

        double prob_mult = probs[2] * probs[(p + 1) & 1];
        for (vi_it a_id = action_ids.begin(); a_id != action_ids.end(); a_id ++)
        {
            pair<int, int> decision_id = make_pair(is_id, *a_id);
            double action_prob = strategy[decision_id];
            double backup_prob = probs[p];
            probs[p] *= action_prob;

            game -> makeAction(*a_id);
            utility temp_util = walkTree(probs);
            double res_util [2] = {temp_util.first, temp_util.second};
            game -> unmakeAction(*a_id);

            probs[p] = backup_prob;

            R[decision_id] += res_util[p] * prob_mult;
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
            R[make_pair(is_id, *a_id)] -= add;
    }
    return final_util;
}


int Cfr::getActionId(int information_set_id, vector<int> action_ids)
{
    double random_double = ((double) rand() / (RAND_MAX));
    double prob_sum = 0;
    int choice = -1;
    for (int i = 0; i < action_ids.size(); i++)
    {
        int action_id = action_ids[i];
        double prob = strategy[make_pair(information_set_id, action_id)];
        prob_sum += prob;
        //printf("Prob of action %d: %0.3f\n", action_id, prob);
        if (choice == -1 && random_double < prob_sum)
        {
            choice = action_id;
        }
    }
    return choice;
}

double Cfr::recomputeStrategy(Smap &reg)
{
    map<int, double> is_r_sums;
    map<int, double> is_r_cnt;
    int zero_cnt = 0, nonzero_cnt = 0, proportional_cnt = 0;
    for (Sit iter = reg.begin(); iter != reg.end(); iter++)
    {
        int is_id = iter -> first.first;
        double val = max(iter -> second, 0.0);

        if (!is_r_sums.count(is_id))
            is_r_sums[is_id] = val;
        else
            is_r_sums[is_id] = is_r_sums[is_id] + val;

        if (!is_r_cnt.count(is_id))
            is_r_cnt[is_id] = 1.0;
        else
            is_r_cnt[is_id] = is_r_cnt[is_id] + 1.0;
    }

    double max_regret = 0.0;
    for (Sit iter = reg.begin(); iter != reg.end(); iter++)
    {
        int is_id = iter -> first.first;
        int a_id = iter -> first.second;
        double val = max(iter -> second, 0.0);
        max_regret = max(val, max_regret);
        double sum = is_r_sums[is_id];
        if (sum > 0.0)
        {
            strategy[iter -> first] = val / sum;
            if (val == 0.0)
                zero_cnt ++;
            else
                nonzero_cnt ++;
        }
        else
        {
            strategy[iter -> first] = 1.0 / is_r_cnt[is_id];
            proportional_cnt ++;
        }
    }
    printf("ZERO CNT: %d, NON ZERO CNT: %d PROPORTIONAL_CNT: %d\n", zero_cnt, nonzero_cnt, proportional_cnt);
    return max_regret;
}


bool Cfr::loadFromFile(const char* filename)
{
    try {
        FILE *f = fopen(filename, "r");
        if (f == NULL)
            return false;
        int strategy_size;
        fscanf(f, "%d\n", &strategy_size);
        for (int i = 0 ; i < strategy_size; i++)
        {
            int is_id, action_id;
            double prob;
            fscanf(f, "%d %d %lf\n", &is_id, &action_id, &prob);
            strategy[make_pair(is_id, action_id)] = prob;
        }
        fclose(f);
        return true;
    }
    catch (int e)
    {
        return false;
    }
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

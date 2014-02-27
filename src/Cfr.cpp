#include "Cfr.h"
#include "GameAbstraction.h"

using namespace std;

const char* Cfr::DEFAULT_FILE = "cfr.strategy";
const char Cfr::FILE_DELIM = '\n';
const int Cfr::ITERATIONS = 1000;

Cfr::Cfr(GameAbstraction* gm)
{
    printf("Init Cfr\n");
    game = gm;
    if (!loadFromFile(DEFAULT_FILE))
    {
        srand(time(0));
        computeVanillaCfr();
        saveToFile(DEFAULT_FILE);
    }
}

void Cfr::computeVanillaCfr()
{
    printf("Computing CFR strategy\n");
    initialized = false;
    for (int i = 0; i < ITERATIONS; i++)
    {
        printf("Iteration %d\n", i);
        double probs [3] = {1.0, 1.0, 1.0};
        walkTree(probs);
        initialized = true;
        recomputeStrategy(R);
    }
    recomputeStrategy(S);
}

utility Cfr::walkTree(double probs[3])
{
    //printf("BEFORE: probs0: %f, probs1: %f, probs2: %f\n", probs[0], probs[1], probs[2]);
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

        double prob_mult = probs[2];
        for (int i = 0; i < 2; i ++)
            if (p != i)
                prob_mult *= probs[i];

        double psum = 0.0;
        for (vi_it a_id = action_ids.begin(); a_id != action_ids.end(); a_id ++)
        {
            pair<int, int> decision_id = make_pair(is_id, *a_id);
            double action_prob = strategy[decision_id];
            psum += action_prob;
            //printf("action_prob: %f\n", action_prob);
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
        if (psum < 0.999)
            printf("PSUM TOO SMALL: %f\n", psum);

        double add = prob_mult;
        if (p == 0)
            add *= final_util.first;
        if (p == 1)
            add *= final_util.second;

        for (vi_it a_id = action_ids.begin(); a_id != action_ids.end(); a_id ++)
        {
            pair<int, int> decision_id = make_pair(is_id, *a_id);
            //printf("ADD: %f\n", add);
            R[decision_id] -= add;
        }
    }
    //printf("AFTER: probs0: %f, probs1: %f, probs2: %f\n", probs[0], probs[1], probs[2]);
    return final_util;
}


int Cfr::getActionId(int information_set_id, vector<int> action_ids)
{
    double random_double = ((double) rand() / (RAND_MAX));
    double prob_sum = 0;
    for (int i = 0; i < action_ids.size(); i++)
    {
        int action_id = action_ids[i];
        double prob = strategy[make_pair(information_set_id, action_id)];
        prob_sum += prob;
        printf("Prob of action %d: %0.3f\n", action_id, prob);
        if (random_double < prob_sum)
            return action_id;
    }
    return -1;
}

void Cfr::recomputeStrategy(Smap &reg)
{
    map<int, double> is_r_sums;
    map<int, double> is_r_cnt;
    int zero_cnt = 0, nonzero_cnt = 0;
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
    for (Sit iter = reg.begin(); iter != reg.end(); iter++)
    {
        int is_id = iter -> first.first;
        int a_id = iter -> first.second;
        double val = max(iter -> second, 0.0);
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
            nonzero_cnt ++;
        }
    }
    printf("ZERO CNT: %d, NON ZERO CNT: %d\n", zero_cnt, nonzero_cnt);
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

#include <set>
#include <fstream>
#include "Cfr.h"
#include "GameAbstraction.h"

using namespace std;

const char* Cfr::DEFAULT_FILE = "cfr.stg";
const char Cfr::FILE_DELIM = '\n';
const int Cfr::ITERATIONS = 2000;

int cnt = 0;

Cfr::Cfr(GameAbstraction* gm, int iterations, const char* strategy_file, bool samp)
{
    printf("Init Cfr, iterations: %d\n", iterations);
    sampled = samp;
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
    current_regret_sum = 0.0;
    double sum = 0.0;
    for (int i = 0; i < iterations; i++)
    {
        printf("Iteration %d\n", i);
        double probs [3] = {1.0, 1.0, 1.0};
        cnt = 0;
        newR.clear();
        walkTree(probs);
        recomputeStrategy(newR);
        double it_err = current_regret_sum / (i + 1);
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
    //printf("%d\n", R.size());

    /* If it is a turn of a chance player */
    if (p == RANDOM_PLAYER_NR)
    {
        log(2, "walkTree: cnt: %d\n", cnt);
        dist action_distr = game -> getActionDistribution();
        double backup_prob = probs[RANDOM_PLAYER_NR];

        if (sampled)
        {
            double r = (double) rand() / RAND_MAX;
            double sum = 0.0;
            int l = 0;
            for (dist_it iter = action_distr.begin(); iter != action_distr.end(); iter++)
            {
                sum += iter -> second;

                if (r < sum + 1e-9)
                {
                    probs[RANDOM_PLAYER_NR] *= iter -> second;
                    game -> makeAction(iter -> first);
                    utility res_util = walkTree(probs);
                    game -> unmakeAction(iter -> first);
                    probs[RANDOM_PLAYER_NR] = backup_prob;
                    final_util.first = res_util.first;
                    final_util.second = res_util.second;
                    break;
                }
                l ++;
            }
        }
        else
        {
            //printf("RAND ACTIONS: %d\n", (int)action_distr.size());
            for (dist_it iter = action_distr.begin(); iter != action_distr.end(); iter++)
            {
                probs[RANDOM_PLAYER_NR] *= iter -> second;
                game -> makeAction(iter -> first);
                utility res_util = walkTree(probs);
                game -> unmakeAction(iter -> first);
                probs[RANDOM_PLAYER_NR] = backup_prob;
                final_util.first += res_util.first * iter -> second;
                final_util.second += res_util.second * iter -> second;
            }
        }
    }
    else
    {
        int is_id = game -> getInformationSetId();
        vector<int> action_ids = game -> getActionIds();
        for (vi_it a_id = action_ids.begin(); a_id != action_ids.end(); a_id ++)
        {
            pair<int, int> decision_id = make_pair(is_id, *a_id);
            if (!strategy.count(decision_id))
                strategy[decision_id] = 1.0 / action_ids.size();
            if (!R.count(decision_id))
            {
                //printf("R: is_id: %d, R.size: %d\n", is_id, (int)R.size());
                R[decision_id] = 0.0;
            }
            if (!S.count(decision_id))
                S[decision_id] = 0.0;
        }

        double prob_mult = probs[RANDOM_PLAYER_NR] * probs[(p + 1) & 1];
        double backup_prob = probs[p];
        for (vi_it a_id = action_ids.begin(); a_id != action_ids.end(); a_id ++)
        {
            pair<int, int> decision_id = make_pair(is_id, *a_id);
            double action_prob = strategy[decision_id];
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
        {
            pair<int, int> decision_id = make_pair(is_id, *a_id);
            R[decision_id] -= add;
            newR[decision_id] = R[decision_id];
        }
    }
    return final_util;
}

int Cfr::getActionId(dist action_dist)
{
    for (int i = 0; i < action_dist.size(); i++)
        printf("a_id: %d, prob: %lf\n", action_dist[i].first, action_dist[i].second);

    double random_double = ((double) rand() / (RAND_MAX));
    double prob_sum = 0;
    int choice = -1;
    for (int i = 0; i < action_dist.size(); i++)
    {
        int action_id = action_dist[i].first;
        double prob = action_dist[i].second;
        prob_sum += prob;
        if (random_double < prob_sum)
            return action_id;
    }
    return action_dist[0].first;

}

int Cfr::getActionId(int information_set_id, vector<int> action_ids)
{
    dist action_dist;
    for (int i = 0; i < action_ids.size(); i++)
    {
        int action_id = action_ids[i];
        pair<int,int> pair_id = make_pair(information_set_id, action_id);
        if (strategy.count(pair_id))
            action_dist.push_back(make_pair(action_id, strategy[pair_id]));
    }
    return getActionId(action_dist);
}

int Cfr::getActionId(dist information_set_ids, vector<int> action_ids)
{
    map<int, double> probs;
    double total_sum = 0.0;
    for (int i = 0; i < action_ids.size(); i++)
    {
        int action_id = action_ids[i];
        double sum = 0.0;
        for (int j = 0; j < information_set_ids.size(); j++)
        {
            int information_set_id = information_set_ids[j].first;
            pair<int,int> pair_id = make_pair(information_set_id, action_id);
            if (strategy.count(pair_id))
            {
                printf("is: %d, a: %d, prob: %.03lf, weight: %.03lf\n", information_set_id, action_id,
                                                        strategy[pair_id], information_set_ids[j].second);
                sum += strategy[pair_id] * information_set_ids[j].second;
            }
        }
        probs[action_ids[i]] = sum;
        total_sum += sum;
    }
    dist action_dist;
    if (total_sum > 0.0)
    {
        for (int i = 0; i < action_ids.size(); i++)
        {
            int action_id = action_ids[i];
            if (probs.count(action_id))
                action_dist.push_back(make_pair(action_id, probs[action_id] / total_sum));
        }
        return getActionId(action_dist);
    }
    else
    {
        printf("WARNING: DEFAULT ACTION CHOSEN\n");
        return action_ids[0];
    }
}

double Cfr::recomputeStrategy(Smap &reg)
{
    map<int, double> is_r_sums;
    map<int, double> is_r_cnt;
    map<int, double> mregret;
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

        if (!mregret.count(is_id))
            mregret[is_id] = val;
        else
            mregret[is_id] = max(mregret[is_id], val);
    }

    for (map<int, double>::iterator it = mregret.begin(); it != mregret.end(); it++)
    {
        int is_id = it -> first;
        double val = it -> second;
        if(!regrets.count(is_id))
        {
            regrets[is_id] = val;
            current_regret_sum += val;
        }
        else
        {
            current_regret_sum += val - regrets[is_id];
            regrets[is_id] = val;
        }
    }

    double max_regret = 0.0;
    set<int> isets;
    for (Sit iter = reg.begin(); iter != reg.end(); iter++)
    {
        int is_id = iter -> first.first;
        isets.insert(is_id);
        int a_id = iter -> first.second;
        double val = max(iter -> second, 0.0);
        max_regret = max(val, max_regret);
        double sum = is_r_sums[is_id];
        if (sum > 0.0)
            strategy[iter -> first] = val / sum;
        else
            strategy[iter -> first] = 1.0 / is_r_cnt[is_id];
    }
    printf("TOTAL_SIZE: %d, IS_SIZE: %d\n", (int)R.size(), (int)isets.size());
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

#include <set>
#include <fstream>
#include "Cfr.h"
#include "GameAbstraction.h"

using namespace std;

const char* Cfr::DEFAULT_FILE = "cfr.stg";
const char Cfr::FILE_DELIM = '\n';
const int Cfr::ITERATIONS = 2000;

int cnt = 0;
bool first_iteration = true;
bool sampled = false;
map<pair<long long, int>, utility> util_map;

inline int _rand_choice(int n)
{
    return rand() % n;
}

Cfr::Cfr(GameAbstraction* gm, int iterations, const char* strategy_file, bool information_tree)
{
    printf("Init Cfr, iterations: %d\n", iterations);
    itree = information_tree;
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
    srand(time(0));
    printf("Computing CFR strategy\n");
    current_regret_sum = 0.0;
    double sum = 0.0;
    for (int i = 0; i < iterations; i++)
    {
        printf("Iteration %d\n", i);
        long double probs [3] = {1.0L, 1.0L, 1.0L};
        cnt = 0;
        newR.clear();
        walkTree(probs);
        printf("TREE SIZE: %d\n", cnt);
        recomputeStrategy(newR);
        double it_err = current_regret_sum / (i + 1);
        sum += it_err;
        printf("It err: %0.5f Err: %0.5f\n", it_err, sum / (i+1));
        first_iteration = false;
        sampled = true;
    }
    recomputeStrategy(S);
}

utility Cfr::walkTree(long double probs[3])
{
    cnt ++;
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
        long double backup_prob = probs[RANDOM_PLAYER_NR];
        int choice = _rand_choice(action_distr.size());
        int i = 0;
        utility res_util;
        for (dist_it iter = action_distr.begin(); iter != action_distr.end(); iter++)
        {
            probs[RANDOM_PLAYER_NR] *= (long double) iter -> second;
            game -> makeAction(iter -> first);
            long long is_id = game -> getInformationSetId();
            pair<long long, int> rpair = make_pair(is_id, iter -> first);
            if (!sampled || !util_map.count(rpair) || choice == i)
            {
                res_util = walkTree(probs);
                util_map[rpair] = res_util;
            }
            else
                res_util = util_map[rpair];
            game -> unmakeAction(iter -> first);
            probs[RANDOM_PLAYER_NR] = backup_prob;
            final_util.first += res_util.first * iter -> second;
            final_util.second += res_util.second * iter -> second;
            i ++;
        }
    }
    else
    {
        long long is_id = game -> getInformationSetId();
        vector<int> action_ids = game -> getActionIds();
        if (first_iteration)
            for (vi_it a_id = action_ids.begin(); a_id != action_ids.end(); a_id ++)
            {
                pair<long long, int> decision_id = make_pair(is_id, *a_id);
                if (!strategy.count(decision_id))
                {
                    strategy[decision_id] = 1.0 / action_ids.size();
                    R[decision_id] = 0.0;
                    S[decision_id] = 0.0;
                }
            }

        long double prob_mult = probs[RANDOM_PLAYER_NR] * probs[(p + 1) & 1];
        long double backup_prob = probs[p];
        for (vi_it a_id = action_ids.begin(); a_id != action_ids.end(); a_id ++)
        {
            pair<long long, int> decision_id = make_pair(is_id, *a_id);
            long double action_prob = (long double) strategy[decision_id];
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
            pair<long long, int> decision_id = make_pair(is_id, *a_id);
            R[decision_id] -= add;
            newR[decision_id] = R[decision_id];
        }
    }
    return final_util;
}

int Cfr::getActionId(dist action_dist, double total_prob_sum)
{
    for (int i = 0; i < action_dist.size(); i++)
        printf("a_id: %d, prob: %lf\n", action_dist[i].first, action_dist[i].second);

    double random_double = ((double) rand() / (RAND_MAX)) * total_prob_sum;
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

int Cfr::getActionId(long long information_set_id, vector<int> action_ids)
{
    dist action_dist;
    double joined_prob;
    for (int i = 0; i < action_ids.size(); i++)
    {
        int action_id = action_ids[i];
        pair<long long, int> pair_id = make_pair(information_set_id, action_id);
        if (strategy.count(pair_id))
        {
            double prob = strategy[pair_id];
            action_dist.push_back(make_pair(action_id, prob));
            joined_prob += prob;

        }
    }
    if (!action_dist.size())
        return action_ids[0];
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
            long long information_set_id = information_set_ids[j].first;
            pair<long long, int> pair_id = make_pair(information_set_id, action_id);
            if (strategy.count(pair_id))
            {
                printf("is: %lld, a: %d, prob: %.03lf, weight: %.03lf\n", information_set_id, action_id,
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
    map<long long, double> is_r_sums;
    map<long long, double> is_r_cnt;
    map<long long, double> mregret;
    for (Sit iter = reg.begin(); iter != reg.end(); iter++)
    {
        long long is_id = iter -> first.first;
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

    for (map<long long, double>::iterator it = mregret.begin(); it != mregret.end(); it++)
    {
        long long is_id = it -> first;
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
        long long is_id = iter -> first.first;
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
            long long is_id;
            int action_id;
            double prob;
            fscanf(f, "%lld %d %lf\n", &is_id, &action_id, &prob);
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
    for (Sit iter = strategy.begin(); iter != strategy.end(); iter++)
    {
        long long is_id = iter -> first.first;
        int action_id = iter -> first.second;
        double prob = iter -> second;
        fprintf(f, "%lld %d %lf%c", is_id, action_id, prob, FILE_DELIM);
    }
    fclose(f);
}

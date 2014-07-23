#include <set>
#include <fstream>
#include <queue>
#include "ModCfr.h"
#include "GameAbstraction.h"

using namespace std;

const char* ModCfr::DEFAULT_FILE = "mod-cfr.stg";
const char ModCfr::FILE_DELIM = '\n';
const int ModCfr::ITERATIONS = 2000;

ModCfr::ModCfr(GameAbstraction* gm, int iterations, const char* strategy_file)
{
    printf("Init Mod Cfr, iterations: %d\n", iterations);
    game = gm;
    double err_sum = 0.0;
    if (!loadFromFile(strategy_file))
    {
        printf("Mod Cfr: Building Tree\n");
        start_is = _exploreTree();
        in_edges[start_is] = 0;
        _topo_order_isets();
        for (int it = 0; it < iterations; it++)
        {
            printf("Mod Cfr, Iteration %d\n", it);
            _recomputeRegrets();
            printf("Regrets recomputed\n");
            double it_err = _recomputeStrategy(R) / (it + 1);
            err_sum += it_err;
            printf("It err: %0.5f Err: %0.5f\n", it_err, err_sum / (it + 1));

        }
        _recomputeStrategy(S);
        srand(time(0));
        saveToFile(strategy_file);
    }
}

void ModCfr::_recomputeRegrets()
{
    _probsBfs();
    visited.clear();
    _walkTree(start_is);
}

void ModCfr::_topo_order_isets()
{
    queue<int> q;
    q.push(start_is);
    while(!q.empty())
    {
        int is_id = q.front();
        q.pop();
        isets_topo_ordered.push_back(is_id);
        Nlist nlist = is_graph[is_id];
        for (int i = 0; i < nlist.size(); i++)
        {
            int n_id = nlist[i].first.first;
            if(!is_final.count(n_id))
            {
                in_edges[n_id] -= 1;
                if (!in_edges[n_id])
                    q.push(n_id);
            }
        }
    }
    //checking
    map<int, bool> vis;
    for (int i = 0; i < isets_topo_ordered.size(); i++)
    {
        int is_id = isets_topo_ordered[i];
        vis[is_id] = true;
        Nlist nlist = is_graph[is_id];
        for (int j = 0; j < nlist.size(); j++)
        {
            int n_id = nlist[j].first.first;
            if (vis.count(n_id))
                printf("ERRROR\n");
        }
    }
}

void ModCfr::_probsBfs()
{
    for (int i = 0; i < all_isets.size(); i++)
    {
        int is_id = all_isets[i];
        for (int j = 0; j < 4; j++)
            probs[j][is_id] = 0.0;
    }
    for (int j = 0; j < 4; j++)
        probs[j][start_is] = 1.0;

    for (int i = 0; i < isets_topo_ordered.size(); i++)
    {
        int is_id = isets_topo_ordered[i];
        int pnum = is_player[is_id];
        Nlist nlist = is_graph[is_id];
        double pbs[4] = {probs[0][is_id], probs[1][is_id], probs[2][is_id], probs[3][is_id]};

        for (int j = 0; j < nlist.size(); j++)
        {
            int n_id = nlist[j].first.first;
            if (pnum == 2)
            {
                double prob = nlist[j].second;
                probs[0][n_id] += pbs[0];
                probs[1][n_id] += pbs[1];
                probs[2][n_id] += pbs[2] * prob;
                probs[3][n_id] += pbs[3] * prob;
            }
            else
            {
                int a_id = nlist[j].first.second;
                pair<int, int> decision_id = make_pair(is_id, a_id);
                double prob = strategy[decision_id];
                int oth = (pnum + 1) & 1;
                probs[oth][n_id] += pbs[oth];
                probs[oth + 2][n_id] += pbs[oth + 2];
                probs[pnum][n_id] += pbs[pnum] * prob;
                probs[pnum + 2][n_id] += pbs[pnum + 2] * prob;
            }
        }
    }
}

utility ModCfr::_walkTree(int is_id)
{
    if (is_final.count(is_id) || visited.count(is_id))
        return is_utility[is_id];
    visited[is_id] = true;

    int pnum = is_player[is_id];
    Nlist nlist = is_graph[is_id];
    utility final_util = make_pair(0.0, 0.0);
    double pbs[4] = {probs[0][is_id], probs[1][is_id], probs[2][is_id], probs[3][is_id]};

    //printf("Walk Tree %d player: %d\n", is_id, pnum);
    for (int i = 0; i < nlist.size(); i++)
    {
        int n_id = nlist[i].first.first;
        int a_id = nlist[i].first.second;
        pair<int, int> decision_id = make_pair(is_id, a_id);

        double prob;
        if (pnum == 2)
            prob = nlist[i].second;
        else
            prob = strategy[decision_id];

        utility n_util = _walkTree(n_id);
        //printf("    neigbour %d, prob: %lf : <%lf, %lf>\n", n_id, prob, n_util.first, n_util.second);
        final_util.first += n_util.first * prob;
        final_util.second += n_util.second * prob;

        if (pnum == 0)
        {
            R[decision_id] += pbs[3] * n_util.first;
            S[decision_id] += pbs[0] * prob;
        }
        else if (pnum == 1)
        {
            R[decision_id] += pbs[2] * n_util.second;
            S[decision_id] += pbs[1] * prob;
        }
    }
    if (pnum < 2)
        for (int i = 0; i < nlist.size(); i++)
        {
            int a_id = nlist[i].first.second;
            pair<int, int> decision_id = make_pair(is_id, a_id);
            if (pnum == 0)
                R[decision_id] -= final_util.first * pbs[3];
            else
                R[decision_id] -= final_util.second * pbs[2];
        }

    is_utility[is_id] = final_util;
    //printf("final util: <%lf, %lf>\n", final_util.first, final_util.second);
    return final_util;
}

int ModCfr::_exploreTree()
{
    int is_id = game -> getInformationSetId();
    if (!in_edges.count(is_id))
        in_edges[is_id] = 1;
    else
        in_edges[is_id] += 1;
    if (visited.count(is_id))
        return is_id;

    visited[is_id] = true;
    all_isets.push_back(is_id);

    if (game -> isFinal())
    {
        is_final[is_id] = true;
        is_utility[is_id] = game -> getUtility();
        return is_id;
    }

    int p = game -> getPlayerId();
    is_player[is_id] = p;

    Nlist nlist;
    if (p == RANDOM_PLAYER_NR)
    {
        dist action_distr = game -> getActionDistribution();
        for (dist_it iter = action_distr.begin(); iter != action_distr.end(); iter++)
        {
            game -> makeAction(iter -> first);
            int n_id = _exploreTree();
            game -> unmakeAction(iter -> first);
            nlist.push_back(make_pair(make_pair(n_id, iter -> first), iter -> second));
        }
    }
    else
    {
        vector<int> action_ids = game -> getActionIds();
        for (vi_it a_id = action_ids.begin(); a_id != action_ids.end(); a_id ++)
        {
            pair<int, int> decision_id = make_pair(is_id, *a_id);
            strategy[decision_id] = 1.0 / action_ids.size();
            R[decision_id] = 0.0;
            S[decision_id] = 0.0;
            game -> makeAction(*a_id);
            int n_id = _exploreTree();
            game -> unmakeAction(*a_id);
            nlist.push_back(make_pair(make_pair(n_id, *a_id), 0.0));
        }
    }
    is_graph[is_id] = nlist;
    return is_id;
}

double ModCfr::_recomputeStrategy(Smap &reg)
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

    double regret_sum = 0;
    for (map<int, double>::iterator it = mregret.begin(); it != mregret.end(); it++)
        regret_sum += it -> second;

    for (Sit iter = reg.begin(); iter != reg.end(); iter++)
    {
        int is_id = iter -> first.first;
        double val = max(iter -> second, 0.0);
        double sum = is_r_sums[is_id];
        if (sum > 0.0)
            strategy[iter -> first] = val / sum;
        else
            strategy[iter -> first] = 1.0 / is_r_cnt[is_id];
    }
    printf("TOTAL_SIZE: %d, IS_SIZE: %d\n", (int)reg.size(), (int)mregret.size());
    return regret_sum;
}

int ModCfr::getActionId(dist action_dist)
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

int ModCfr::getActionId(int information_set_id, vector<int> action_ids)
{
    dist action_dist;
    for (int i = 0; i < action_ids.size(); i++)
    {
        int action_id = action_ids[i];
        pair<int,int> pair_id = make_pair(information_set_id, action_id);
        if (strategy.count(pair_id))
            action_dist.push_back(make_pair(action_id, strategy[pair_id]));
    }
    if (!action_dist.size())
        return action_ids[0];
    return getActionId(action_dist);
}

int ModCfr::getActionId(dist information_set_ids, vector<int> action_ids)
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



bool ModCfr::loadFromFile(const char* filename)
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
void ModCfr::saveToFile(const char* filename)
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

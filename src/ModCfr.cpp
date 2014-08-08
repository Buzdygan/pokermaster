#include <set>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <cstring>
#include <queue>
#include "ModCfr.h"
#include "GameAbstraction.h"

using namespace std;

const char* ModCfr::DEFAULT_FILE = "mod-cfr.stg";
const char ModCfr::FILE_DELIM = '\n';
const int ModCfr::ITERATIONS = 2000;


double tab_R[MAX_ISETS][5];
double tab_S[MAX_ISETS][5];
double tab_strategy[MAX_ISETS][5];

map<int, int> state_to_id;
int id_to_state[MAX_STATES];

map<int, int> is_to_id;
int id_to_is[MAX_ISETS];
vector<int> is_actions[MAX_ISETS];

int state_to_is[MAX_STATES];

Nlist graph[MAX_STATES];
int state_player[MAX_STATES];
bool is_final[MAX_STATES];

bool visited[MAX_STATES];
utility state_utility[MAX_STATES];
double probs[4][MAX_STATES];

int state_cnt = 1;
int is_cnt = 1;

ModCfr::ModCfr(HoldemPokerModAbstraction* gm, int iterations, const char* strategy_file_template)
{
    printf("Init Mod Cfr, iterations: %d\n", iterations);
    game = gm;
    double err_sum = 0.0;
    memset(is_final, false, sizeof(is_final));
    char strategy_file [100];
    sprintf(strategy_file, "%s-%d.stg", strategy_file_template, iterations);
    if (!loadFromFile(strategy_file))
    {
        printf("Mod Cfr: Building Tree\n");
        start_state = _exploreTree();
        printf("Mod Cfr: Tree Built\n");
        in_edges[start_state] = 0;
        printf("Mod Cfr: Topo ordering\n");
        _topo_order_states();
        printf("Mod Cfr: Topo ordered\n");
        for (int it = 0; it < iterations; it++)
        {
            printf("Mod Cfr, Iteration %d\n", it);
            _recomputeRegrets();
            printf("Regrets recomputed\n");
            double it_err = _recomputeStrategy(tab_R) / (it + 1);
            err_sum += it_err;
            printf("It err: %0.5f Err: %0.5f\n", it_err, err_sum / (it + 1));

            if ((it + 1) % 100000 == 0 && (iterations - it) >= 100000)
            {
                _recomputeStrategy(tab_S);
                sprintf(strategy_file, "%s-%d.stg", strategy_file_template, it);
                _copyStrategy();
                saveToFile(strategy_file);
                _recomputeStrategy(tab_R);
            }

        }
        sprintf(strategy_file, "%s-%d.stg", strategy_file_template, iterations);
        _recomputeStrategy(tab_S);
        _copyStrategy();
        saveToFile(strategy_file);
    }
}

void ModCfr::_recomputeRegrets()
{
    _probsBfs();
    memset(visited, false, sizeof(visited));
    _walkTree(start_state);
}

void ModCfr::_topo_order_states()
{
    queue<int> q;
    q.push(start_state);
    while(!q.empty())
    {
        int s_id = q.front();
        q.pop();
        states_topo_ordered.push_back(s_id);
        Nlist nlist = graph[s_id];
        for (int i = 0; i < nlist.size(); i++)
        {
            int n_id = nlist[i].first.first;
            if(!is_final[n_id])
            {
                in_edges[n_id] -= 1;
                if (!in_edges[n_id])
                    q.push(n_id);
            }
        }
    }
    //checking
    map<int, bool> vis;
    for (int i = 0; i < states_topo_ordered.size(); i++)
    {
        int s_id = states_topo_ordered[i];
        vis[s_id] = true;
        Nlist nlist = graph[s_id];
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
    for (int i = 0; i < all_states.size(); i++)
    {
        int s_id = all_states[i];
        for (int j = 0; j < 4; j++)
            probs[j][s_id] = 0.0;
    }
    for (int j = 0; j < 4; j++)
        probs[j][start_state] = 1.0;

    for (int i = 0; i < states_topo_ordered.size(); i++)
    {
        int s_id = states_topo_ordered[i];
        int is_id = state_to_is[s_id];
        int pnum = state_player[s_id];
        Nlist nlist = graph[s_id];
        double pbs[4] = {probs[0][s_id], probs[1][s_id], probs[2][s_id], probs[3][s_id]};

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
                double prob = tab_strategy[is_id][a_id];
                int oth = (pnum + 1) & 1;
                probs[oth][n_id] += pbs[oth];
                probs[oth + 2][n_id] += pbs[oth + 2];
                probs[pnum][n_id] += pbs[pnum] * prob;
                probs[pnum + 2][n_id] += pbs[pnum + 2] * prob;
            }
        }
    }
}

utility ModCfr::_walkTree(int s_id)
{
    if (is_final[s_id] || visited[s_id])
        return state_utility[s_id];
    visited[s_id] = true;

    int is_id = state_to_is[s_id];
    int pnum = state_player[s_id];
    Nlist nlist = graph[s_id];
    utility final_util = make_pair(0.0, 0.0);
    double pbs[4] = {probs[0][s_id], probs[1][s_id], probs[2][s_id], probs[3][s_id]};

    //printf("Walk Tree %d player: %d\n", s_id, pnum);
    for (int i = 0; i < nlist.size(); i++)
    {
        int n_id = nlist[i].first.first;
        int a_id = nlist[i].first.second;

        double prob;
        if (pnum == 2)
            prob = nlist[i].second;
        else
            prob = tab_strategy[is_id][a_id];

        utility n_util = _walkTree(n_id);
        //printf("    neigbour %d, prob: %lf : <%lf, %lf>\n", n_id, prob, n_util.first, n_util.second);
        final_util.first += n_util.first * prob;
        final_util.second += n_util.second * prob;

        if (pnum == 0)
        {
            tab_R[is_id][a_id] += pbs[3] * n_util.first;
            tab_S[is_id][a_id] += pbs[0] * prob;
        }
        else if (pnum == 1)
        {
            tab_R[is_id][a_id] += pbs[2] * n_util.second;
            tab_S[is_id][a_id] += pbs[1] * prob;
        }
    }
    if (pnum < 2)
        for (int i = 0; i < nlist.size(); i++)
        {
            int a_id = nlist[i].first.second;
            if (pnum == 0)
                tab_R[is_id][a_id] -= final_util.first * pbs[3];
            else
                tab_R[is_id][a_id] -= final_util.second * pbs[2];
        }

    state_utility[s_id] = final_util;
    //printf("final util: <%lf, %lf>\n", final_util.first, final_util.second);
    return final_util;
}

int ModCfr::_exploreTree()
{
    int game_s_id = game -> getStateId();
    int game_is_id = game -> getISetId();
    int s_id, is_id;
    int p = game -> getPlayerId();
    //printf("explore Tree, game_s_id: %d\n", game_s_id);
    if(!state_to_id.count(game_s_id))
    {
        s_id = state_cnt;
        id_to_state[state_cnt] = game_s_id;
        state_to_id[game_s_id] = state_cnt ++;
    }
    else
        s_id = state_to_id[game_s_id];

    // first occurence of this information set
    if(!is_to_id.count(game_is_id))
    {
        is_id = is_cnt;
        id_to_is[is_cnt] = game_is_id;
        is_to_id[game_is_id] = is_cnt ++;

        // init regrets and strategy
        if (p != RANDOM_PLAYER_NR)
        {
            player_isets.push_back(is_id);
            vector<int> action_ids = game -> getActionIds();
            is_actions[is_id] = action_ids;
            for (vi_it a_id = action_ids.begin(); a_id != action_ids.end(); a_id ++)
            {
                tab_strategy[is_id][*a_id] = 1.0 / action_ids.size();
                tab_R[is_id][*a_id] = 0.0;
                tab_S[is_id][*a_id] = 0.0;
            }
        }
    }
    else
        is_id = is_to_id[game_is_id];
    state_to_is[s_id] = is_id;
    //printf("explore Tree, game_s_id: %d, s_id: %d, state_cnt: %d\n", game_s_id, s_id, state_cnt);

    if (!in_edges.count(s_id))
        in_edges[s_id] = 1;
    else
        in_edges[s_id] += 1;
    if (visited[s_id])
        return s_id;


    visited[s_id] = true;
    all_states.push_back(s_id);

    if (game -> isFinal())
    {
        is_final[s_id] = true;
        state_utility[s_id] = game -> getUtility();
        return s_id;
    }

    state_player[s_id] = p;

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
            game -> makeAction(*a_id);
            int n_id = _exploreTree();
            game -> unmakeAction(*a_id);
            nlist.push_back(make_pair(make_pair(n_id, *a_id), 0.0));
        }
    }
    graph[s_id] = nlist;
    return s_id;
}

double ModCfr::_recomputeStrategy(double reg[MAX_ISETS][5])
{
    printf("recompute Strategy R\n");
    double is_r_sums [MAX_ISETS];
    double is_r_cnt [MAX_ISETS];
    double mregret [MAX_ISETS];
    memset(is_r_sums, 0.0, sizeof(is_r_sums));
    memset(is_r_cnt, 0.0, sizeof(is_r_cnt));
    memset(mregret, 0.0, sizeof(mregret));

    double regret_sum = 0;

    printf("check1, player_isets.size: %d\n", (int)player_isets.size());
    for (int i = 0; i < player_isets.size(); i++)
    {
        int is_id = player_isets[i];
        for (int j = 0; j < is_actions[is_id].size(); j++)
        {
            int a_id = is_actions[is_id][j];
            double r = reg[is_id][a_id];
            //printf("is_id: %d, a_id: %d, r: %lf\n", is_id, a_id, r);
            double val = max(r, 0.0);
            is_r_sums[is_id] = is_r_sums[is_id] + val;
            is_r_cnt[is_id] = is_r_cnt[is_id] + 1.0;
            regret_sum -= mregret[is_id];
            mregret[is_id] = max(mregret[is_id], val);
            regret_sum += mregret[is_id];

        }
    }
    for (int i = 0; i < player_isets.size(); i++)
    {
        int is_id = player_isets[i];
        for (int j = 0; j < is_actions[is_id].size(); j++)
        {
            int a_id = is_actions[is_id][j];
            double r = reg[is_id][a_id];
            double val = max(r, 0.0);
            double sum = is_r_sums[is_id];
            if (sum > 0.0)
                tab_strategy[is_id][a_id] = val / sum;
            else
                tab_strategy[is_id][a_id] = 1.0 / is_r_cnt[is_id];
        }
    }
    printf("end recompute Strategy\n");
    return regret_sum;
}

void ModCfr::_copyStrategy()
{
    for (int i = 0; i < player_isets.size(); i++)
    {
        int is_id = player_isets[i];
        for (int j = 0; j < is_actions[is_id].size(); j++)
        {
            int a_id = is_actions[is_id][j];
            pair<int, int> decision_id = make_pair(id_to_is[is_id], a_id);
            strategy[decision_id] = tab_strategy[is_id][a_id];
        }
    }
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

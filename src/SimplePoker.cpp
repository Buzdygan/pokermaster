#include "SimplePoker.h"

SimplePoker::SimplePoker()
{
}

SimplePoker::~SimplePoker()
{
}

int SimplePoker::getInformationSetId()
{
    return -1;
}
utility SimplePoker::getUtility()
{
    return make_pair(-1.0, -1.0);
}
int SimplePoker::getPlayerId()
{
    return -1;
}
dist SimplePoker::getActionDistribution()
{
    vector<pair<int, double> > v;
    return v;
}
bool SimplePoker::isFinal()
{
    return false;
}
vector<int> SimplePoker::getActionIds()
{
    return vector<int> ();
}
void SimplePoker::makeAction(int action_id)
{

}
void SimplePoker::unmakeAction(int action_id)
{

}

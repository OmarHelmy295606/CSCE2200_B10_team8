#include "../include/ds/BDDStorage.hpp"
#include "../include/ds/stack.hpp"
#include <cmath>

BDDStorage::BDDStorage(int numVars): n(numVars)
{
    nodes.push_back({n, 0, 0});
    nodes.push_back({n, 1, 1});
}

int BDDStorage::mk(int v, int l, int h)
{
    if(l == h) return l;

    BDDNode node(v,l,h);
    int* id = uniqueTable.find(node);

    if(!id)
    {
        int nid = nodes.size();
        nodes.push_back(node);
        uniqueTable.insert(node, nid);
        return nid;
    }
    return *id;
}

int BDDStorage::variable(int v)
{
    return mk(v, 0, 1);
}

int BDDStorage::notvariable(int v)
{
    return mk(v, 1, 0);
}

int BDDStorage::getVar(int id) const
{
    return nodes[id].var;
}

int BDDStorage::getLow(int id) const
{
    return nodes[id].low;
}

int BDDStorage::getHigh(int id) const
{
    return nodes[id].high;
}

bool BDDStorage::isTerminal(int id) const
{
    return nodes[id].high == nodes[id].low;
}

int BDDStorage::size() const
{
    return nodes.size();
}

int BDDStorage::numVars() const
{
    return n;
}

int BDDStorage::fromTruthTable(const Bitset& tt)
{
    int rows = pow(2,n);
    vec<int> level;
    for(int i=0; i<rows; i++)
        level.push_back(tt.test(i)? 1:0);

    for(int v=n-1; v>=0; v--)
    {
        rows /=2;
        for(int i=0; i<rows; i++)
        {
            level[i] = mk(v,level[i*2], level[i*2+1]);
        }
    }
    return level[0];  //this is the id of the root 
}

bool BDDStorage::evaluate(int root, int row) const
{
    while(!isTerminal(root))
    {
        int var = nodes[root].var;
        int next = (row >> n-1-var) &1;
        root = (next)? nodes[root].high: nodes[root].low;
    }
    return root==1;
}

int BDDStorage::countReachable(int root) const
{
    Bitset visitedNodes(size());
    stack<int> toVisit;
    int count = 0;

    visitedNodes.set(root);
    toVisit.push(root);
    count++;

    while(!toVisit.empty())
    {
        root = toVisit.top();
        toVisit.pop();

        int children[2] = {nodes[root].low, nodes[root].high};
        for(int c:children)
        {
            if(!visitedNodes.test(c))
            {
                count++;
                visitedNodes.set(c);
                toVisit.push(c);
            }
        }
    }
    return count;
}


#include "../include/ds/BDDStorage.hpp"

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


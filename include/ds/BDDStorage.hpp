#ifndef BDDSTORAGE_HPP
#define BDDSTORAGE_HPP

#include "../ds_Bitset.h"
#include "vec.hpp"
#include "hashMap.hpp"

struct BDDNode
{
     int var, low, high; 


     BDDNode(int v=0, int l=0, int h=0): var(v), low(l), high(h){}
     bool operator==(const BDDNode& key)
     {
        return (var == key.var && low == key.low && high == key.high);
     }
};
inline unsigned int hashOf(const BDDNode& key)
{
    unsigned int h = hashOf(key.var);
    h ^= hashOf(key.low)  + 0x9e3779b9u + (h << 6) + (h >> 2);
    h ^= hashOf(key.high) + 0x9e3779b9u + (h <<6) + (h >> 2);
    return h;
}

class BDDStorage 
{
    private:
        vec<BDDNode> nodes;
        hashMap<BDDNode, int> uniqueTable;
        int n; //numVariables
    public:
        explicit BDDStorage(int numVars); 
        int mk(int var, int low, int high); 
        int variable(int i); 
        int notvariable(int i);

        int getVar(int id) const; 
        int getLow(int id) const; 
        int getHigh(int id) const;
        bool isTerminal(int id) const;
        int size() const; 
        int numVars() const;

        int fromTruthTable(const Bitset& tt);
        bool evaluate(int root, int row) const;
        int countReachable(int root) const; 
};

#endif

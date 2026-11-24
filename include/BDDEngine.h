#ifndef BDDENGINE_HPP
#define BDDENGINE_HPP

#include <stdexcept>
#include "ds/BDDStorage.hpp"
#include "ds/stack.hpp"
#include "ds/vec.hpp"
#include "ds/hashMap.hpp"
#include "Expression.h"

struct IteKey
{
    int f, g, h;
    IteKey(int f, int g, int h) : f(f), g(g), h(h) {}
};

bool operator==(const IteKey &a, const IteKey &b);
unsigned int hashOf(const IteKey &k);

struct PostfixToken
{
    enum Kind { VAR, CONST, NOT, AND, OR, XOR };
    Kind kind = CONST;
    int value = 0;
    PostfixToken() {}
    PostfixToken(Kind k, int v = 0) : kind(k), value(v) {}
};

class BDDEngine
{
private:
    BDDStorage store;
    hashMap<IteKey, int> computed;
    long long hits;
    long long misses;

    int cofactor(int x, int v, bool b) const;

public:
    BDDEngine(int numVars);

    int ite(int f, int g, int h);
    int bddNot(int f);
    int bddAnd(int f, int g);
    int bddOr(int f, int g);
    int bddXor(int f, int g);

    int fromExpression(const vec<PostfixToken> &postfix);
    int fromExpression(const Expression &expression);
    bool equivalent(int f, int g) const;
    bool findOne(int f, vec<int> &assignment) const;
    unsigned long long satCount(int root) const;

    BDDStorage &storage();
    const BDDStorage &storage() const;
    int numVars();
    long long cacheHits();
    long long cacheMisses();
    int cacheSize() const;
    void clearCache();
};

#endif
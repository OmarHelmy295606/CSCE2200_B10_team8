#include "../include/BDDEngine.h"
#include "../include/BDDCount.h"

bool operator==(const IteKey &a, const IteKey &b)
{
    return a.f == b.f && a.g == b.g && a.h == b.h;
}

unsigned int hashOf(const IteKey &k)
{
    unsigned int h = hashOf(k.f);
    h ^= hashOf(k.g) + 0x9e3779b9u + (h << 6) + (h >> 2);
    h ^= hashOf(k.h) + 0x9e3779b9u + (h << 6) + (h >> 2);
    return h;
}

BDDEngine::BDDEngine(int numVars) : store(numVars), hits(0), misses(0) {}

int BDDEngine::cofactor(int x, int v, bool b) const
{
    if (store.isTerminal(x) || store.getVar(x) != v)
        return x;
    return b ? store.getHigh(x) : store.getLow(x);
}

int BDDEngine::ite(int f, int g, int h)
{
    // terminal cases
    if (f == 1)
        return g;
    if (f == 0)
        return h;
    if (g == h)
        return g;
    if (g == 1 && h == 0)
        return f;

    IteKey key(f, g, h);
    int *found = computed.find(key);
    if (found)
    {
        hits++;
        return *found;
    }
    misses++;

    // the top variable is the smallest one of the three (terminals have var = n, so they never win)
    int v = store.getVar(f);
    if (store.getVar(g) < v)
        v = store.getVar(g);
    if (store.getVar(h) < v)
        v = store.getVar(h);

    int t = ite(cofactor(f, v, true), cofactor(g, v, true), cofactor(h, v, true));
    int e = ite(cofactor(f, v, false), cofactor(g, v, false), cofactor(h, v, false));
    int r = store.mk(v, e, t);

    computed.insert(key, r);
    return r;
}

int BDDEngine::bddNot(int f) { return ite(f, 0, 1); }
int BDDEngine::bddAnd(int f, int g) { return ite(f, g, 0); }
int BDDEngine::bddOr(int f, int g) { return ite(f, 1, g); }
int BDDEngine::bddXor(int f, int g) { return ite(f, bddNot(g), g); }

int BDDEngine::fromExpression(const vec<PostfixToken> &postfix)
{
    stack<int> ids;
    for (size_t i = 0; i < postfix.size(); i++)
    {
        const PostfixToken &t = postfix[i];
        if (t.kind == PostfixToken::VAR)
        {
            if (t.value < 0 || t.value >= store.numVars())
                throw std::invalid_argument("variable out of range in the expression");
            ids.push(store.variable(t.value));
        }
        else if (t.kind == PostfixToken::CONST)
        {
            ids.push(t.value ? 1 : 0);
        }
        else if (t.kind == PostfixToken::NOT)
        {
            if (ids.empty())
                throw std::invalid_argument("NOT without an operand");
            ids.push(bddNot(ids.pop()));
        }
        else
        {
            if (ids.size() < 2)
                throw std::invalid_argument("binary operator without two operands");
            int b = ids.pop();
            int a = ids.pop();
            if (t.kind == PostfixToken::AND)
                ids.push(bddAnd(a, b));
            else if (t.kind == PostfixToken::OR)
                ids.push(bddOr(a, b));
            else
                ids.push(bddXor(a, b));
        }
    }
    if (ids.size() != 1)
        throw std::invalid_argument("the expression does not reduce to one value");
    return ids.pop();
}

int BDDEngine::fromExpression(const Expression &expression)
{
    const vec<Token>& tokens = expression.postfix();
    vec<PostfixToken> postfix;

    for (size_t i = 0; i < tokens.size(); i++) {
        PostfixToken::Kind kind;

        switch (tokens[i].type) {
            case TOK_VAR:   kind = PostfixToken::VAR;   break;
            case TOK_CONST: kind = PostfixToken::CONST; break;
            case TOK_NOT:   kind = PostfixToken::NOT;   break;
            case TOK_AND:   kind = PostfixToken::AND;   break;
            case TOK_OR:    kind = PostfixToken::OR;    break;
            case TOK_XOR:   kind = PostfixToken::XOR;   break;
            default: throw std::invalid_argument("Unexpected postfix token");
        }

        postfix.push_back(PostfixToken(kind, tokens[i].value));
    }

    return fromExpression(postfix);
}

bool BDDEngine::equivalent(int f, int g) const
{
    return f == g;
}

bool BDDEngine::findOne(int f, vec<int> &assignment) const
{
    assignment = vec<int>(store.numVars(), 0);
    if (f == 0)
        return false;

    // every node other than 0 reaches 1, so we only have to avoid stepping onto 0
    int id = f;
    while (!store.isTerminal(id))
    {
        int v = store.getVar(id);
        if (store.getLow(id) != 0)
        {
            assignment[v] = 0;
            id = store.getLow(id);
        }
        else
        {
            assignment[v] = 1;
            id = store.getHigh(id);
        }
    }
    return true;
}
unsigned long long BDDEngine::satCount(int root) const
{
    return ::satCount(store, root);
}
BDDStorage &BDDEngine::storage() { return store; }
const BDDStorage &BDDEngine::storage() const { return store; }
int BDDEngine::numVars() { return store.numVars(); }

long long BDDEngine::cacheHits() { return hits; }
long long BDDEngine::cacheMisses() { return misses; }
int BDDEngine::cacheSize() const { return computed.size(); }

void BDDEngine::clearCache()
{
    computed.clear();
    hits = 0;
    misses = 0;
}

void BDDEngine::writeDot(int root, const VarTable& vars, std::ostream& out) const
{
    stack<int> pending;
    Bitset visited(store.size());

    int numVars = store.numVars();
    vec<vec<int>> levels(numVars + 1);  // one row per variable, plus the terminals

    pending.push(root);
    visited.set(root);

    out << "digraph BDD {\n";

    while (!pending.empty()) {
        int id = pending.pop();
        int level = store.getVar(id);
        levels[level].push_back(id);

        if (store.isTerminal(id)) {
            out << "  n" << id << " [label=\"" << id << "\", shape=box];\n";
            continue;
        }

        int low = store.getLow(id);
        int high = store.getHigh(id);

        out << "  n" << id << " [label=\"" << vars.name(level) << "\"];\n";
        out << "  n" << id << " -> n" << low  << " [style=dashed];\n";
        out << "  n" << id << " -> n" << high << " [style=solid];\n";

        int children[2] = {low, high};
        for (int i = 0; i < 2; i++) {
            int child = children[i];
            if (!visited.test(child)) {
                visited.set(child);
                pending.push(child);
            }
        }
    }

    // tell Graphviz which nodes should be drawn on the same row
    for (int level = 0; level <= numVars; level++) {
        if (levels[level].empty())
            continue;

        out << "  { rank=same;";
        for (size_t i = 0; i < levels[level].size(); i++)
            out << " n" << levels[level][i] << ";";
        out << " }\n";
    }

    out << "}\n";
}
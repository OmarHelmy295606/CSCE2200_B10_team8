#include <iostream>
#include <string>
#include <stdexcept>
#include "../include/BDDEngine.h"
using namespace std;

int passed = 0;
int failed = 0;

void check(bool ok, const string& name) {
    if (ok) {
        passed++;
    } else {
        failed++;
        cout << "FAILED: " << name << endl;
    }
}

// evaluates a postfix string on one row without any BDD (variable 0 is the MSB of the row)
bool evalPostfix(const string& text, int n, int row) {
    vec<PostfixToken> p = BDDEngine::parsePostfix(text);
    stack<bool> s;
    for (size_t i = 0; i < p.size(); i++) {
        PostfixToken t = p[i];
        if (t.kind == PostfixToken::VAR) s.push((row >> (n - 1 - t.value)) & 1);
        else if (t.kind == PostfixToken::CONST) s.push(t.value == 1);
        else if (t.kind == PostfixToken::NOT) s.push(!s.pop());
        else {
            bool b = s.pop();
            bool a = s.pop();
            if (t.kind == PostfixToken::AND) s.push(a && b);
            else if (t.kind == PostfixToken::OR) s.push(a || b);
            else s.push(a != b);
        }
    }
    return s.pop();
}

Bitset truthTableOf(const string& text, int n) {
    Bitset tt(1 << n);
    for (int r = 0; r < (1 << n); r++)
        if (evalPostfix(text, n, r)) tt.set(r);
    return tt;
}

int buildPostfix(BDDEngine& e, const string& text) {
    return e.fromExpression(BDDEngine::parsePostfix(text));
}

int rowOf(const vec<int>& a) {
    int r = 0;
    for (size_t i = 0; i < a.size(); i++) r = (r << 1) | a[i];
    return r;
}

int main() {
    // ---------- terminal cases of ite ----------
    {
        BDDEngine e(3);
        int a = e.storage().variable(0);
        int b = e.storage().variable(1);
        check(e.ite(1, a, b) == a, "ite(1, g, h) = g");
        check(e.ite(0, a, b) == b, "ite(0, g, h) = h");
        check(e.ite(a, b, b) == b, "ite(f, g, g) = g");
        check(e.ite(a, 1, 0) == a, "ite(f, 1, 0) = f");
        check(e.cacheHits() == 0 && e.cacheMisses() == 0, "terminal cases do not touch the cache");
    }

    // ---------- single operations ----------
    {
        BDDEngine e(2);
        int a = e.storage().variable(0);
        int b = e.storage().variable(1);
        check(e.bddNot(a) == e.storage().notvariable(0), "NOT A is the node (A, 1, 0)");
        check(e.bddNot(e.bddNot(a)) == a, "NOT NOT A = A");
        check(e.bddAnd(a, 0) == 0 && e.bddAnd(a, 1) == a, "A AND 0 = 0, A AND 1 = A");
        check(e.bddOr(a, 1) == 1 && e.bddOr(a, 0) == a, "A OR 1 = 1, A OR 0 = A");
        check(e.bddXor(a, a) == 0, "A XOR A = 0");
        check(e.bddOr(a, e.bddNot(a)) == 1, "A OR NOT A = 1");
        check(e.bddAnd(a, b) == e.bddAnd(b, a), "AND is commutative (same node)");
        check(e.bddXor(a, b) == e.bddXor(b, a), "XOR is commutative (same node)");
    }

    // ---------- ITE gives the same node as fromTruthTable ----------
    {
        const char* exprs[] = {
            "AB&C|",            // AB + C
            "AB&A'C&|",         // AB + A'C (a multiplexer)
            "AB&AC&|BC&|",      // majority of 3
            "AB^C^D^",          // parity of 4
            "AB|'CD&|",         // (A+B)' + CD
            "AD&B'C&|E^",       // (AD + B'C) xor E
            "AA'&",             // contradiction
            "AA'|B&",           // B written the long way
            "1A&",              // A with a constant
            "AB&CD&|EF&|GH&|",  // 8 variables, four products
        };
        int count = sizeof(exprs) / sizeof(exprs[0]);
        for (int i = 0; i < count; i++) {
            string s = exprs[i];
            vec<PostfixToken> p = BDDEngine::parsePostfix(s);
            int n = BDDEngine::variablesUsed(p);
            BDDEngine e(n);
            int fromIte = e.fromExpression(p);
            int fromTable = e.storage().fromTruthTable(truthTableOf(s, n));
            check(fromIte == fromTable, s + ": ITE and fromTruthTable give the same node");

            bool rowsOk = true;
            for (int r = 0; r < (1 << n); r++)
                if (e.storage().evaluate(fromIte, r) != evalPostfix(s, n, r)) rowsOk = false;
            check(rowsOk, s + ": the BDD evaluates like the expression on every row");
        }
    }

    // ---------- the computed table ----------
    {
        BDDEngine e(4);
        int f = buildPostfix(e, "AB&C|");
        int g = buildPostfix(e, "CD^");
        long long hitsBefore = e.cacheHits();
        long long missesBefore = e.cacheMisses();
        int first = e.bddAnd(f, g);
        long long missesAfterFirst = e.cacheMisses();
        check(missesAfterFirst > missesBefore, "a new operation is a cache miss");
        long long hitsAfterFirst = e.cacheHits();
        int second = e.bddAnd(f, g);
        check(second == first, "repeating the operation gives the same node");
        check(e.cacheHits() == hitsAfterFirst + 1, "repeating the operation is one cache hit");
        check(e.cacheMisses() == missesAfterFirst, "repeating the operation adds no miss");
        check(e.cacheHits() >= hitsBefore, "hits never go down");

        // building the same expression again only hits the cache
        long long m = e.cacheMisses();
        int f2 = buildPostfix(e, "AB&C|");
        check(f2 == f, "building the same expression twice gives the same node");
        check(e.cacheMisses() == m, "building it again has no misses");

        e.clearCache();
        check(e.cacheSize() == 0 && e.cacheHits() == 0 && e.cacheMisses() == 0, "clearCache empties the table");
        check(e.bddAnd(f, g) == first, "after clearCache the result is still the same node");
    }

    // ---------- equivalence ----------
    {
        const char* same[][2] = {
            {"AB&'", "A'B'|"},              // De Morgan
            {"AB|'", "A'B'&"},              // De Morgan
            {"ABC|&", "AB&AC&|"},           // distributive
            {"AB^", "AB'&A'B&|"},           // XOR definition
            {"AB&A'C&|BC&|", "AB&A'C&|"},   // consensus
            {"AAB&|", "A"},                 // absorption
            {"AB^C^", "CA^B^"},             // XOR in another order
        };
        for (int i = 0; i < 7; i++) {
            string x = same[i][0], y = same[i][1];
            BDDEngine e(3);
            int f = buildPostfix(e, x);
            int g = buildPostfix(e, y);
            check(e.equivalent(f, g), x + " and " + y + " are equivalent");
            vec<int> a;
            check(!e.findOne(e.bddXor(f, g), a), x + " and " + y + ": their XOR has no 1");
        }

        const char* different[][2] = {
            {"AB|", "AB^"},
            {"AB&C|", "ABC|&"},
            {"A", "B"},
            {"AB&AC&|BC&|", "AB^C^"},
            {"AB&", "0"},
        };
        for (int i = 0; i < 5; i++) {
            string x = different[i][0], y = different[i][1];
            BDDEngine e(3);
            int f = buildPostfix(e, x);
            int g = buildPostfix(e, y);
            check(!e.equivalent(f, g), x + " and " + y + " are not equivalent");
            vec<int> a;
            bool found = e.findOne(e.bddXor(f, g), a);
            check(found, x + " and " + y + ": findOne finds an input");
            int r = rowOf(a);
            check(found && evalPostfix(x, 3, r) != evalPostfix(y, 3, r),
                  x + " and " + y + ": the two differ on that input");
        }
    }

    // ---------- findOne ----------
    {
        BDDEngine e(3);
        vec<int> a;
        check(!e.findOne(0, a), "findOne(0) finds nothing");
        check(e.findOne(1, a) && a.size() == 3, "findOne(1) gives an input of 3 variables");
        int f = buildPostfix(e, "A'BC'&&");
        check(e.findOne(f, a) && a[0] == 0 && a[1] == 1 && a[2] == 0, "findOne(A'BC') is 010");
        int g = buildPostfix(e, "AB&C&");
        check(e.findOne(g, a) && rowOf(a) == 7, "findOne(ABC) is 111");
    }

    // ---------- malformed postfix ----------
    {
        BDDEngine e(2);
        bool threw = false;
        try { buildPostfix(e, "A&"); } catch (const invalid_argument&) { threw = true; }
        check(threw, "an operator without two operands is rejected");
        threw = false;
        try { buildPostfix(e, "AB"); } catch (const invalid_argument&) { threw = true; }
        check(threw, "two values with no operator are rejected");
        threw = false;
        try { buildPostfix(e, "AC&"); } catch (const invalid_argument&) { threw = true; }
        check(threw, "a variable past numVars is rejected");
        threw = false;
        try { BDDEngine::parsePostfix("A#B"); } catch (const invalid_argument&) { threw = true; }
        check(threw, "an unknown character is rejected");
    }

    cout << passed << " passed, " << failed << " failed" << endl;
    return failed == 0 ? 0 : 1;
}

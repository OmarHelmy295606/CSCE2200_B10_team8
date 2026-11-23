#include <iostream>
#include "../include/Expression.h"
using namespace std;

int passed = 0;
int failed = 0;

void check(bool ok, const char* name) {
    if (ok) {
        passed++;
    } else {
        failed++;
        cout << "FAILED: " << name << endl;
    }
}

// helper: parses two expressions with the same VarTable and checks if their truth tables match
bool sameTruthTable(const string& s1, const string& s2) {
    VarTable vars;
    Expression e1, e2;
    if (!e1.parse(s1, vars)) return false;
    if (!e2.parse(s2, vars)) return false;
    Bitset t1 = e1.truthTable(vars.size());
    Bitset t2 = e2.truthTable(vars.size());
    return t1.equals(t2);
}

// helper: checks that an invalid expression is rejected and gives an error message
bool rejectedWithMessage(const string& s) {
    VarTable vars;
    Expression e;
    bool ok = e.parse(s, vars);
    return !ok && !e.error().empty();
}

int main() {
    // 1. VarTable tests
    {
        VarTable vt;
        check(vt.size() == 0, "new VarTable is empty");
        check(vt.add("A") == 0, "first variable gets index 0");
        check(vt.add("B") == 1, "second variable gets index 1");
        check(vt.add("A") == 0, "duplicate variable keeps index 0");
        check(vt.size() == 2, "VarTable size stays 2 after duplicate");
        check(vt.indexOf("B") == 1, "indexOf finds B at 1");
        check(vt.indexOf("Z") == -1, "indexOf returns -1 for missing variable");
        check(vt.name(0) == "A" && vt.name(1) == "B", "name(i) returns correct names");
    }

    // 2. Variable naming rules (x1y1 = 2 vars, A0 = 1 var)
    {
        VarTable vars;
        Expression e;
        check(e.parse("x1y1", vars), "x1y1 parses cleanly");
        check(vars.size() == 2, "x1y1 has 2 variables");
        check(vars.name(0) == "x1" && vars.name(1) == "y1", "x1y1 splits into x1 and y1");
    }
    {
        VarTable vars;
        Expression e;
        check(e.parse("A0", vars), "A0 parses cleanly");
        check(vars.size() == 1, "A0 has 1 variable");
        check(vars.name(0) == "A0", "A0 stays a single variable named A0");
    }

    // 3. Truth table rows for "AB + A'C + BC" (minterms 1, 3, 6, 7)
    {
        VarTable vars;
        Expression e;
        check(e.parse("AB + A'C + BC", vars), "AB + A'C + BC parses cleanly");
        check(vars.size() == 3, "AB + A'C + BC has 3 variables");
        Bitset tt = e.truthTable(vars.size());
        check(tt.size() == 8, "3-var truth table has 8 rows");
        check(tt.count() == 4, "AB + A'C + BC has 4 minterms");
        check(!tt.test(0) && tt.test(1) && !tt.test(2) && tt.test(3) &&
              !tt.test(4) && !tt.test(5) && tt.test(6) && tt.test(7),
              "AB + A'C + BC minterms are 1, 3, 6, 7");
    }

    // 4. Algebraic laws: De Morgan, XOR, Absorption
    check(sameTruthTable("(A+B)'", "A'B'"), "De Morgan 1: (A+B)' == A'B'");
    check(sameTruthTable("(AB)'", "A'+B'"), "De Morgan 2: (AB)' == A'+B'");
    check(sameTruthTable("A^B", "A'B + AB'"), "XOR: A^B == A'B + AB'");
    check(sameTruthTable("A + AB", "A"), "Absorption: A + AB == A");

    // 5. Operator synonyms, double NOT, implicit AND, constants
    check(sameTruthTable("!A", "A'"), "prefix ! equals postfix '");
    check(sameTruthTable("~A", "A'"), "prefix ~ equals postfix '");
    check(sameTruthTable("!!A", "A"), "double prefix NOT cancels out");
    check(sameTruthTable("A''", "A"), "double postfix NOT cancels out");
    check(sameTruthTable("A & B | C", "A . B + C"), "&/| equals ./+");
    check(sameTruthTable("A(B + C)", "AB + AC"), "implicit AND before '('");
    check(sameTruthTable("(A + B)(A + C)", "A + BC"), "implicit AND between ')('");
    check(sameTruthTable("A!B", "AB'"), "implicit AND before '!'");
    check(sameTruthTable("A + 0", "A"), "identity A + 0 == A");
    check(sameTruthTable("A * 1", "A"), "identity A * 1 == A");
    {
        VarTable vars;
        Expression e0, e1;
        check(e0.parse("0", vars) && e0.truthTable(0).count() == 0, "constant 0 evaluates to 0");
        check(e1.parse("1", vars) && e1.truthTable(0).count() == 1, "constant 1 evaluates to 1");
    }

    // 6. Invalid expressions rejected with error messages
    check(rejectedWithMessage("A+"), "rejects trailing operator: A+");
    check(rejectedWithMessage("(A"), "rejects missing closing paren: (A");
    check(rejectedWithMessage("A#B"), "rejects invalid character: A#B");
    check(rejectedWithMessage(""), "rejects empty string");
    check(rejectedWithMessage("   "), "rejects whitespace-only string");
    check(rejectedWithMessage("()"), "rejects empty parentheses: ()");
    check(rejectedWithMessage("A)"), "rejects extra closing paren: A)");
    check(rejectedWithMessage("+A"), "rejects leading binary operator: +A");
    check(rejectedWithMessage("'A"), "rejects leading postfix NOT: 'A");
    check(rejectedWithMessage("A+*B"), "rejects consecutive binary operators: A+*B");

    cout << passed << " passed, " << failed << " failed" << endl;
    return 0;
}

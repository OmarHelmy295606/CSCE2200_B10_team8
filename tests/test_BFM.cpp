#include <iostream>
#include <fstream>
#include <string>
#include "../include/BooleanFunctionMinimizer.h"
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

// loads a file and runs the whole minimization, returns false if the file is rejected
bool minimizeFile(BFM& f, const string& path) {
    if (!f.loadBF(path)) return false;
    f.generatePrimeImplicants();
    f.generateEPIs();
    f.greedyCover();
    f.generateExpressions();
    return true;
}

// the number of product terms in the first line of an expected file (the exact minimum)
int expectedTerms(const string& path) {
    ifstream in(path);
    string line;
    getline(in, line);
    int terms = 1;
    for (size_t i = 0; i < line.size(); i++)
        if (line[i] == '+') terms++;
    return terms;
}

int main(int argc, char* argv[]) {
    string cases = (argc > 1) ? argv[1] : "TestCasesQM";

    // every valid file: loaded, a correct cover, and not smaller than the exact minimum
    const char* valid[] = {
        "01_column1_only", "02_up_to_column2", "03_up_to_column3", "04_up_to_column4",
        "05_all_minterms", "06_only_PI", "07_one_and_two_ones", "08_three_and_four_ones",
        "09_maxterm_conversion", "10_row_column_dominance", "11_petrick", "12_ten_vars_long",
        "13_petrick_cyclic_two_solutions", "14_petrick_twelve_solutions"
    };
    for (int i = 0; i < 14; i++) {
        string name = valid[i];
        BFM f;
        bool loaded = minimizeFile(f, cases + "/valid/" + name + ".txt");
        check(loaded, name + ": the file is loaded");
        if (!loaded) continue;
        check(f.solutionIsCorrect(), name + ": the greedy solution covers the function");
        check(f.solutionSize() >= expectedTerms(cases + "/expected/" + name + ".txt"),
              name + ": not fewer terms than the exact minimum");
    }

    // results known by hand
    BFM column1;
    minimizeFile(column1, cases + "/valid/01_column1_only.txt");
    check(column1.getExpression() == "A'B'C'D'", "01: m0 of 4 variables is A'B'C'D'");

    BFM all;
    minimizeFile(all, cases + "/valid/05_all_minterms.txt");
    check(all.getExpression() == "1", "05: all minterms give 1");

    BFM maxterms;
    minimizeFile(maxterms, cases + "/valid/09_maxterm_conversion.txt");
    check(maxterms.solutionSize() == 2, "09: the maxterm input gives 2 terms");

    BFM cyclic;
    minimizeFile(cyclic, cases + "/valid/13_petrick_cyclic_two_solutions.txt");
    check(cyclic.primeImplicantsCount() == 6, "13: the cyclic function has 6 prime implicants");
    check(cyclic.essentialPrimeImplicantsCount() == 0, "13: none of them is essential");
    check(cyclic.solutionSize() == 3 && cyclic.solutionLiterals() == 6, "13: greedy finds 3 terms with 6 literals");

    // a function where greedy is not minimal (the exact answer has 4 terms, 9 literals)
    BFM greedy;
    check(minimizeFile(greedy, cases + "/greedy/01_greedy_not_minimal.txt"), "greedy example: the file is loaded");
    check(greedy.solutionIsCorrect(), "greedy example: the cover is still correct");
    check(greedy.solutionSize() == 5, "greedy example: greedy takes 5 terms");

    // invalid files are rejected
    const char* invalid[] = {
        "01_only_dont_cares", "02_out_of_range", "03_duplicate_minterm", "04_mixed_m_and_M", "05_spaces"
    };
    cout << "(the 6 ERROR lines below are expected)" << endl;
    for (int i = 0; i < 5; i++) {
        string name = invalid[i];
        BFM f;
        check(!f.loadBF(cases + "/invalid/" + name + ".txt"), name + ": the invalid file is rejected");
    }
    BFM missing;
    check(!missing.loadBF(cases + "/no_such_file.txt"), "a missing file is rejected");

    cout << passed << " passed, " << failed << " failed" << endl;
    return 0;
}

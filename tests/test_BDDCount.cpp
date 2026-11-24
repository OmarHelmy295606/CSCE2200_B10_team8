#include <iostream>
#include "../include/BDDCount.h"

int passed = 0;
int failed = 0;

void check(bool ok, const char* name) {
    if (ok) {
        passed++;
    } else {
        failed++;
        std::cout << "FAILED: " << name << std::endl;
    }
}

int main() {
    const int numVars = 4;
    const int numRows = 1 << numVars;
    BDDStorage store(numVars);

    Bitset alwaysFalse(numRows);
    int falseRoot = store.fromTruthTable(alwaysFalse);
    check(satCount(store, falseRoot) == 0,
          "constant false counts zero rows");
    check(satCount(store, falseRoot) ==
              static_cast<unsigned long long>(alwaysFalse.count()),
          "constant false matches Bitset::count");

    Bitset alwaysTrue(numRows);
    alwaysTrue.setAll();
    int trueRoot = store.fromTruthTable(alwaysTrue);
    check(satCount(store, trueRoot) == 16,
          "constant true counts all 16 rows");
    check(satCount(store, trueRoot) ==
              static_cast<unsigned long long>(alwaysTrue.count()),
          "constant true matches Bitset::count");

    Bitset onlyA(numRows);
    Bitset onlyC(numRows);
    Bitset aAndC(numRows);

    for (int row = 0; row < numRows; row++) {
        bool a = ((row >> 3) & 1) != 0;
        bool c = ((row >> 1) & 1) != 0;

        if (a)
            onlyA.set(row);
        if (c)
            onlyC.set(row);
        if (a && c)
            aAndC.set(row);
    }

    int aRoot = store.fromTruthTable(onlyA);
    int cRoot = store.fromTruthTable(onlyC);
    int aAndCRoot = store.fromTruthTable(aAndC);

    check(satCount(store, aRoot) == 8,
          "A counts 8 rows while B, C, and D are skipped");
    check(satCount(store, aRoot) ==
              static_cast<unsigned long long>(onlyA.count()),
          "A matches Bitset::count");

    check(satCount(store, cRoot) == 8,
          "C counts 8 rows while A and B are skipped before the root");
    check(satCount(store, cRoot) ==
              static_cast<unsigned long long>(onlyC.count()),
          "C matches Bitset::count");

    check(satCount(store, aAndCRoot) == 4,
          "A AND C counts 4 rows while B and D are skipped");
    check(satCount(store, aAndCRoot) ==
              static_cast<unsigned long long>(aAndC.count()),
          "A AND C matches Bitset::count");

    std::cout << passed << " passed, " << failed << " failed" << std::endl;
    return failed == 0 ? 0 : 1;
}


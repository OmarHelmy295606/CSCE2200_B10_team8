#include <iostream>
#include "../include/ds_Bitset.h"
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

int main() {

    Bitset a(10);
    check(a.count() == 0, "new bitset has count 0");
    check(a.none(), "new bitset: none() is true");

    a.set(3);
    check(a.test(3), "bit 3 is 1 after set(3)");
    check(!a.test(4), "bit 4 is still 0");
    a.reset(3);
    check(!a.test(3), "bit 3 is 0 after reset(3)");

    Bitset b(130);
    b.set(63);
    b.set(64);
    b.set(129);
    check(b.test(63), "bit 63 is set");
    check(b.test(64), "bit 64 is set");
    check(b.test(129), "last bit (129) is set");
    check(!b.test(62), "bit 62 is not set");
    check(!b.test(65), "bit 65 is not set");
    check(b.count() == 3, "count is 3");

    b.flip(63);
    check(!b.test(63), "flip turns 1 into 0");
    b.flip(63);
    check(b.test(63), "flip turns 0 into 1");

    Bitset c(70);
    c.setAll();
    check(c.count() == 70, "setAll on 70 bits gives count 70");

    Bitset x(8);
    Bitset y(8);
    x.set(1); x.set(2); x.set(3);
    y.set(2); y.set(3); y.set(4);

    Bitset t = x;
    t.andWith(y);
    check(t.count() == 2 && t.test(2) && t.test(3), "AND gives {2,3}");

    t = x;
    t.orWith(y);
    check(t.count() == 4, "OR gives {1,2,3,4}");

    t = x;
    t.xorWith(y);
    check(t.count() == 2 && t.test(1) && t.test(4), "XOR gives {1,4}");

    t = x;
    t.andNotWith(y);
    check(t.count() == 1 && t.test(1), "AND-NOT gives {1}");

    check(x.countAnd(y) == 2, "countAnd of x and y is 2");

    Bitset small(8);
    small.set(2);
    small.set(3);
    check(small.isSubsetOf(x), "{2,3} is a subset of {1,2,3}");
    check(!x.isSubsetOf(small), "{1,2,3} is not a subset of {2,3}");
    check(x.equals(x), "a set equals itself");
    check(!x.equals(y), "x and y are different");

    Bitset d = x;
    d.set(7);
    check(d.test(7), "copy has the new bit");
    check(!x.test(7), "original does not have it");

    cout << passed << " passed, " << failed << " failed" << endl;
    return 0;
}
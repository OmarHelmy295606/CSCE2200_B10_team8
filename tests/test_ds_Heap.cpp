#include <iostream>
#include "../include/ds/heap.hpp"
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
    // test initial empty state
    heap<int> h;
    check(h.empty(), "new heap is empty");
    check(h.size() == 0, "new heap has size 0");

    //test pushing elements and upheap order
    h.push(15);
    check(h.top() == 15, "top is 15 after first push");

    h.push(40);
    check(h.top() == 40, "top becomes 40 after pushing larger value");

    h.push(10);
    h.push(50);
    h.push(30);
    check(h.size() == 5, "size is 5 after pushing 5 elements");
    check(h.top() == 50, "top is 50 (the maximum element)");

    // test popping elements and downheap order ( and it should come out largest to smallest)
    check(h.pop() == 50, "1st pop extracts 50");
    check(h.pop() == 40, "2nd pop extracts 40");
    check(h.pop() == 30, "3rd pop extracts 30");
    check(h.pop() == 15, "4th pop extracts 15");
    check(h.pop() == 10, "5th pop extracts 10");

    check(h.empty(), "heap is empty after popping all elements");

    // test when popping from an empty heap
    bool caughtException = false;
    try {
        h.pop();
    } catch (const std::out_of_range&) {
        caughtException = true;
    }
    check(caughtException, "popping empty heap throws out_of_range");

    cout << passed << " passed, " << failed << " failed" << endl;
    return 0;
}
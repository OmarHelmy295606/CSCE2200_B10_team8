#include <iostream>
#include "../include/ds/stack.hpp"
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

// a small struct to make sure the stack works with more than ints
struct Token {
    char op;
    int val;
};

// takes a const stack: only compiles if top() const exists
int peekConst(const stack<int>& s) {
    return s.top();
}

int main() {
    // test initial empty state
    stack<int> s;
    check(s.empty(), "new stack is empty");
    check(s.size() == 0, "new stack has size 0");

    // test LIFO order
    s.push(1);
    check(s.top() == 1, "top is 1 after first push");
    s.push(2);
    s.push(3);
    check(s.size() == 3, "size is 3 after three pushes");
    check(s.top() == 3, "top is the last pushed value");
    check(s.pop() == 3, "1st pop returns 3");
    check(s.pop() == 2, "2nd pop returns 2");
    check(s.size() == 1, "size is 1 after two pops");
    check(s.pop() == 1, "3rd pop returns 1");
    check(s.empty(), "stack is empty after popping everything");

    // test empty-stack errors
    bool caught = false;
    try { s.pop(); } catch (const std::out_of_range&) { caught = true; }
    check(caught, "pop on empty stack throws out_of_range");

    caught = false;
    try { s.top(); } catch (const std::out_of_range&) { caught = true; }
    check(caught, "top on empty stack throws out_of_range");

    const stack<int> emptyConst;
    caught = false;
    try { emptyConst.top(); } catch (const std::out_of_range&) { caught = true; }
    check(caught, "const top on empty stack throws out_of_range");

    // test top() gives write access and the const version works
    s.push(10);
    s.top() = 20;
    check(s.top() == 20, "top() reference can modify the top element");
    check(peekConst(s) == 20, "top() works on a const stack");
    check(s.size() == 1, "top() does not remove the element");

    // test pushing the stack's own top (the vec reallocation trap)
    stack<int> t;
    t.push(7);
    for (int i = 0; i < 20; i++) {
        t.push(t.top());
    }
    bool allSeven = true;
    while (!t.empty()) {
        if (t.pop() != 7) allSeven = false;
    }
    check(allSeven, "push(top()) is safe across reallocations");

    // test growth past several reallocations
    stack<int> big;
    for (int i = 0; i < 1000; i++) big.push(i);
    check(big.size() == 1000, "size is 1000 after 1000 pushes");
    bool inOrder = true;
    for (int i = 999; i >= 0; i--) {
        if (big.pop() != i) inOrder = false;
    }
    check(inOrder, "1000 elements come back in reverse order");

    // test pre-sized stack
    stack<int> reserved(500);
    check(reserved.empty() && reserved.size() == 0, "pre-sized stack starts empty");
    for (int i = 0; i < 500; i++) reserved.push(i);
    check(reserved.size() == 500 && reserved.top() == 499, "pre-sized stack fills correctly");

    // test clear
    reserved.clear();
    check(reserved.empty(), "clear empties the stack");
    reserved.push(5);
    check(reserved.top() == 5, "stack is usable after clear");

    // test a user-defined type and rvalue push
    stack<Token> tokens;
    tokens.push(Token{'+', 0});
    Token n = {'x', 42};
    tokens.push(n);
    check(tokens.top().val == 42, "struct element pushed by copy");
    Token popped = tokens.pop();
    check(popped.op == 'x' && popped.val == 42, "struct element popped correctly");
    check(tokens.top().op == '+', "rvalue-pushed struct is next");

    // test copy is independent (this crashes if vec cannot copy)
    stack<int> a;
    a.push(1);
    a.push(2);
    stack<int> b = a;
    b.push(3);
    b.top() = 99;
    check(a.size() == 2 && a.top() == 2, "copy does not affect the original");
    check(b.size() == 3 && b.top() == 99, "copy is independent");

    // test copy assignment and self assignment
    stack<int> c;
    c.push(100);
    c = a;
    check(c.size() == 2 && c.top() == 2, "copy assignment replaces contents");
    c = c;
    check(c.size() == 2 && c.top() == 2, "self assignment is safe");

    // test move
    stack<int> d = std::move(a);
    check(d.size() == 2 && d.top() == 2, "move constructor takes the elements");
    check(a.empty(), "moved-from stack is empty");
    a.push(8);
    check(a.top() == 8, "moved-from stack is still usable");

    // test a stack of stacks (needs correct copy/move inside vec)
    stack<stack<int>> nested;
    stack<int> inner;
    inner.push(4);
    inner.push(5);
    nested.push(inner);
    nested.push(inner);
    inner.push(6);
    check(nested.size() == 2 && nested.top().top() == 5, "nested stacks hold independent copies");

    cout << passed << " passed, " << failed << " failed" << endl;
    return 0;
}
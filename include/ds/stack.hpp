#ifndef STACK_HPP
#define STACK_HPP

#include <cstddef>
#include <stdexcept>
#include <utility>
#include "vec.hpp"

template <typename type>
class stack {
private:
    vec<type> data;   

public:
    stack();
    // Creates an empty stack with room for `cap` elements already allocated.
    // Use it when the maximum depth is known so pushing never triggers a reallocation.
    explicit stack(size_t cap);

    void push(const type& val);
    void push(type&& val);

    type pop();
    
    type& top();
    const type& top() const;

    bool empty() const;

    size_t size() const;

    void clear();
};

#include "../../src/stack.cpp"
#endif
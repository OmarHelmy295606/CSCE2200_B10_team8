#include "../include/ds/stack.hpp"

template <typename type>
stack<type>::stack() {}

template <typename type>
stack<type>::stack(size_t cap) { data.reserve(cap); }

template <typename type>
void stack<type>::push(const type& val) { data.push_back(val); }


template <typename type>
void stack<type>::push(type&& val) { data.push_back(std::move(val)); }

template <typename type>
type stack<type>::pop() {
    if (empty()) {
        throw std::out_of_range("Stack is empty!");
    }
    type val = std::move(data.back());   
    data.pop_back();                     
    return val;
}

template <typename type>
type& stack<type>::top() {
    if (empty()) {
        throw std::out_of_range("Stack is empty!");
    }
    return data.back();
}

template <typename type>
const type& stack<type>::top() const {
    if (empty()) {
        throw std::out_of_range("Stack is empty!");
    }
    return data.back();
}

template <typename type>
bool stack<type>::empty() const { return data.empty(); }

template <typename type>
size_t stack<type>::size() const { return data.size(); }

template <typename type>
void stack<type>::clear() { data.clear(); }
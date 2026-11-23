#ifndef HEAP_HPP
#define HEAP_HPP

#include <stdexcept>
#include "vec.hpp"

template <typename type>
class heap {
private:
    vec<type> a; //the dynamic array to store the heap

    void swapNodes(type& x, type& y); //used in upheaping and downheaping

    //used to ensure the requirments of heap data structure 
    // after inserting a new value
    void upheap(size_t k);

    // used after poping an element and placing the last node in the place
    // of the root to restore the properties of heap DS
    void downheap(size_t i);

public:
    //a constrctor that initializes a vector wiith a dummy value
    // so the real inserted values start from index 1
    heap();

    //the insert function that inserts a (for example) a prime implicant 
    // at the last leaf and then upheap to restore order
    void push(const type& val);

    //extracts the root element and then downheap at index 1
    // to restore properties 
    type pop();

    //returns the value of the root
    type top();

    //the number of elemeents in the heap
    size_t size();

    bool empty();
};

#include "../../src/heap.cpp"

#endif
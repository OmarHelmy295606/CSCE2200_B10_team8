#ifndef HEAP_HPP
#define HEAP_HPP

#include <stdexcept>
#include "vec.hpp"

template <typename type>
class heap {
private:
	vec<type> a; //the dynamic array to store the heap

	void swapNodes(type& x, type& y) { //used in upheaping and downheaping
		type temp = x;
		x = y;
		y = temp;
	}

	//used to ensure the requirments of heap data structure 
	// after inserting a new value
	void upheap(size_t k) { 
		if (k <= 1) return;
		size_t parent = k / 2;
		while (k > 1 && a[k] > a[parent]) {
			swapNodes(a[k], a[parent]);
			k = parent;
			parent = k / 2;
		}
	}

	// used after poping an element and placing the last node in the place
	// of the root to restore the properties of heap DS
	void downheap(size_t i) {
		size_t n = size();
		size_t left = 2 * i;
		size_t right = 2 * i + 1;
		size_t maxIdx = i; //a first assumption

		if (left <= n && a[left] > a[maxIdx]) {
			maxIdx = left;
		}
		if (right <= n && a[right] > a[maxIdx]) {
			maxIdx = right;
		}
		if (maxIdx != i) {
			swapNodes(a[i], a[maxIdx]);
			downheap(maxIdx);
		}
	}

public:
	//a constrctor that initializes a vector wiith a dummy value
	// so the real inserted values start from index 1
	heap() {
		a.push_back(type());
	}

	//the insert function that inserts a (for example) a prime implicant 
	// at the last leaf and then upheap to restore order
	void push(const type& val) {
		a.push_back(val);
		upheap(size());
	}

	//extracts the root element and then downheap at index 1
	// to restore properties 
	type pop() {
		if (empty()) {
			throw std::out_of_range("Heap is empty!");
		}

		type rootVal = a[1];
		a[1] = a.back();
		a.pop_back();

		if (!empty()) {
			downheap(1);
		}
		return rootVal;
	}

	//returns the value of the root
	type top() {
		if (!empty()) {
			return a[1];
		}
		else throw std::out_of_range("Heap is empty!");
	}

	//the number of elemeents in the heap
	size_t size() {
		return a.size() - 1; //as we start the heap from index 1
	}

	bool empty() {
		return a.size() <= 1;
	}
};

#endif
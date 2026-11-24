#ifndef BDD_COUNT_HPP
#define BDD_COUNT_HPP

#include "ds/BDDStorage.hpp"

// count the input assignments for which the BDD reaches terminal 1
unsigned long long satCount(const BDDStorage& store, int root);

#endif
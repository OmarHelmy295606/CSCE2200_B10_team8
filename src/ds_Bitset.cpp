#include "../include/ds_Bitset.h"
#include <cassert>

Bitset::Bitset(int nbits){
    assert(nbits >=0);
    nbits_ = nbits;
    nwords_ = ((nbits + 63) / 64);
    words_ = new Word[nwords_];
    for(int k =0;k<nwords_; k++){
        words_[k] = 0;
    }
}

Bitset::Bitset(const Bitset& other){
    nbits_  = other.nbits_;
    nwords_ = other.nwords_;
    words_  = new Word[nwords_];
    for (int k = 0; k < nwords_; k++)
        words_[k] = other.words_[k];
}

Bitset& Bitset::operator=(const Bitset& other){
    if(this == &other){
        return *this;
    }
    Word* newWords = new Word[other.nwords_];
    for(int k=0;k<other.nwords_;k++){
        newWords[k] = other.words_[k];
    }
    delete[] words_;
    words_  = newWords;
    nbits_  = other.nbits_;
    nwords_ = other.nwords_;
    return *this;
}

Bitset::~Bitset(){
    delete[] words_;
}


// single bit operations
int Bitset::size() const {
    return nbits_;
}

void Bitset::set(int i){
    assert(i>= 0 && i<nbits_);
    words_[i / 64] |= (Word(1) << (i % 64));
}

void Bitset::reset(int i) {
    assert(i >= 0 && i < nbits_);
    words_[i / 64] &= ~(Word(1) << (i % 64));
}

void Bitset::flip(int i) {
    assert(i >= 0 && i < nbits_);
    words_[i / 64] ^= (Word(1) << (i % 64));
}

bool Bitset::test(int i) const {
    assert(i >= 0 && i < nbits_);
    return (words_[i / 64] & (Word(1) << (i % 64))) != 0;
}




// whole set operations
void Bitset::clearAll() {
    for (int k = 0; k < nwords_; k++)
        words_[k] = 0;
}

void Bitset::clearTail() {
    int valid = nbits_ % 64;
    if (valid == 0)
        return;
    words_[nwords_ - 1] &= (Word(1) << valid) - 1;
}

void Bitset::setAll(){
    for (int k = 0; k < nwords_; k++)
    {
        words_[k] =~Word(0);
    }
    clearTail();    
}



int Bitset::popcount(Word x){
    int c=0;
    while(x != 0){
        x &= x - 1;
        c++;
    }
    return c;
}

int Bitset::count() const {
    int total = 0;
    for (int k = 0; k < nwords_; k++)
        total += popcount(words_[k]);
    return total;
}

bool Bitset::any() const{
    for (int k = 0; k < nwords_; k++)
        if (words_[k] != 0)
            return true;
    return false;
}

bool Bitset::none() const{
    return !any();
}


// operations done on two sets


void Bitset::andWith(const Bitset& other) {
    assert(nbits_ == other.nbits_);
    for (int k = 0; k < nwords_; k++)
        words_[k] &= other.words_[k];
}

void Bitset::orWith(const Bitset& other) {
    assert(nbits_ == other.nbits_);
    for (int k = 0; k < nwords_; k++)
        words_[k] |= other.words_[k];
}

void Bitset::xorWith(const Bitset& other) {
    assert(nbits_ == other.nbits_);
    for (int k = 0; k < nwords_; k++)
        words_[k] ^= other.words_[k];
}

void Bitset::andNotWith(const Bitset& other) {
    assert(nbits_ == other.nbits_);
    for (int k = 0; k < nwords_; k++)
        words_[k] &= ~other.words_[k];
}

bool Bitset::equals(const Bitset& other) const {
    if (nbits_ != other.nbits_)
        return false;
    for (int k = 0; k < nwords_; k++)
        if (words_[k] != other.words_[k])
            return false;
    return true;
}

bool Bitset::isSubsetOf(const Bitset& other) const {
    assert(nbits_ == other.nbits_);
    for (int k = 0; k < nwords_; k++)
        if ((words_[k] & ~other.words_[k]) != 0)
            return false;
    return true;
}


int Bitset::countAnd(const Bitset& other) const {
    assert(nbits_ == other.nbits_);
    int total = 0;
    for (int k = 0; k < nwords_; k++)
        total += popcount(words_[k] & other.words_[k]);
    return total;
}
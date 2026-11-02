#ifndef BITSET_H
#define BITSET_H
typedef unsigned long long Word;

class Bitset {
public:
    explicit Bitset(int nbits);

    Bitset(const Bitset& other);
    Bitset& operator=(const Bitset& other);
    ~Bitset();

    int size() const;

    void set(int i);
    void reset(int i);
    void flip(int i);
    bool test(int i) const;

    void clearAll();             
    void setAll();
    int  count() const;
    bool any() const;
    bool none() const;

    void andWith(const Bitset& other);
    void orWith(const Bitset& other);
    void xorWith(const Bitset& other);
    void andNotWith(const Bitset& other);
    bool equals(const Bitset& other) const;
    bool isSubsetOf(const Bitset& other) const;
    int  countAnd(const Bitset& other) const;

private:
    Word* words_;
    int   nbits_;
    int   nwords_;

    void clearTail();
    static int popcount(Word x);
};

#endif
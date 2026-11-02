#ifndef VEC_HPP
#define VEC_HPP

#include <stdexcept>

template <typename type>

class vec
{
    private:
    type* first, *last, *end;

    void grow();


    public:
    vec();
    explicit vec(size_t);
    vec(size_t, const type&);

    void push_back(type);
    void pop_back();
    void erase(size_t);


    void insert(size_t, type);

    void clear();

    size_t size() const;
    type& at(size_t);
    type& operator[](size_t);
    bool empty() const;
    type& front();
    type& back();
    ~vec();


};

#include "../../src/vec.cpp"
#endif
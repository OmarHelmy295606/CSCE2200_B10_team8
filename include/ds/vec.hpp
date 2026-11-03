#ifndef VEC_HPP
#define VEC_HPP

#include <stdexcept>

template <typename type>

class vec
{
    private:
    type* first, *last, *end;

    void grow();
    void reallocate(size_t);
    void swap(vec&) noexcept;
    
    public:
    vec();
    explicit vec(size_t);
    vec(size_t, const type&);
    vec(const vec&);
    vec(vec&&) noexcept;
    vec& operator=(vec);

    void push_back(type);
    void pop_back();
    void erase(size_t);


    void insert(size_t, type);

    void clear();

    size_t size() const;
    size_t capacity() const;
    type& at(size_t);
    type& operator[](size_t);
    bool empty() const;
    type& front();
    type& back();
    const type& back() const;
    void reserve(size_t);
    ~vec();


};

#include "../../src/vec.cpp"
#endif
#include "../include/ds/vec.hpp"

template <typename type>
void vec<type>::grow()
{
    size_t oldCap = end - first;
    size_t size = last - first;
    size_t capacity = (oldCap == 0)? 1: oldCap*2;
    
    type* temp = new type[capacity];
    for(size_t i=0; i<size; i++)
        temp[i]= std::move(first[i]);

    delete[] first;
    first = temp;
    last = first + size;
    end = first + capacity;
}

template <typename type>
vec<type>::vec(){first = end = last = nullptr;}

template <typename type>
vec<type>::vec(size_t size){first = new type[size](); end = first + size; last = end;}

template <typename type>
vec<type>::vec(size_t size, const type& val) 
{
    first = new type[size];  
    for(size_t i=0; i<size; i++)
        first[i] = val; 
    end = first + size; 
    last = end;
}

template <typename type>
void vec<type>::push_back(type data)
{
    if(last == end)
        grow();  

    *last = data;
    last++;
}

template <typename type>
void vec<type>::pop_back()
{
    if(last == first)
        return;
    last--;
    *last = type();
}

template <typename type>
void vec<type>::erase(size_t index)
{
    size_t s = size();
    if(index >= s)
        throw std::out_of_range("Vector size was exceeded");
    for(size_t i=index; i<s-1; i++)
    {
        first[i] = std::move(first[i+1]);
    }
    pop_back();
}

template <typename type>
void vec<type>::insert(size_t pos, type val)
{
    size_t s = size();
    if(pos > s)
        throw std::out_of_range("Vector size was exceeded");
    if(last == end)
        grow();
    for(size_t i = s; i>pos; i--)
    {
        first[i] = std::move(first[i-1]);
    }
    last++;
    first[pos] = val;
}

template <typename type>
void vec<type>::clear()
{
    last = first;
}

template <typename type>
size_t vec<type>::size() const
{
    return last-first;
}

template <typename type>
type& vec<type>::at(size_t index) 
{
    if (index>= size())
            throw std::out_of_range("Vector size was exceeded");
    return first[index];
}

template <typename type>
type& vec<type>::operator[](size_t index)
{
    return first[index];
}

template <typename type>
bool vec<type>::empty() const
{
    return first == last;
}

template <typename type>
type& vec<type>::front()
{
    return *first;
}

template <typename type>
type& vec<type>::back()
{
    return *(last-1);
}

template <typename type>
vec<type>::~vec()
{
    delete [] first;
}

#include "../include/ds/vec.hpp"

template <typename type>
void vec<type>::reallocate(size_t newCap)
{
    size_t s = last - first;                 
    type* temp = new type[newCap];           
    for(size_t i=0; i<s; i++)
        temp[i] = std::move(first[i]);       

    delete[] first;                          
    first = temp;
    last = first + s;                       
    end = first + newCap;                   
}

template <typename type>
void vec<type>::grow()
{
    size_t oldCap = end - first;
    reallocate((oldCap == 0)? 1: oldCap*2);  
}

template <typename type>
size_t vec<type>::capacity() const
{
    return end - first;
}

template <typename type>
void vec<type>::reserve(size_t newCap)
{
    if(newCap > capacity())                 
        reallocate(newCap);
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
vec<type>::vec(const vec& other)
{
    size_t s = other.size();
    first = new type[s];                    
    for(size_t i=0; i<s; i++)
        first[i] = other.first[i];           
    last = end = first + s;
}

template <typename type>
vec<type>::vec(vec&& other) noexcept
{
    first = other.first;
    last = other.last;
    end = other.end;
    other.first = other.last = other.end = nullptr; 
}

template <typename type>
vec<type>& vec<type>::operator=(vec other)    // parameter taken BY VALUE
{
    swap(other);
    return *this;
}

template <typename type>
void vec<type>::swap(vec& other) noexcept
{
    type* t;
    t = first; first = other.first; other.first = t;
    t = last;  last  = other.last;  other.last  = t;
    t = end;   end   = other.end;   other.end   = t;
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
const type& vec<type>::back() const
{
    return *(last-1);
}

template <typename type>
vec<type>::~vec()
{
    delete [] first;
}

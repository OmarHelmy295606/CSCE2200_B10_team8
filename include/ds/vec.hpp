#ifndef VEC_HPP
#define VEC_HPP

#include <stdexcept>

template <typename type>

class vec
{
    private:
    type* first, *last, *end;

    

    void grow()
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


    public:
    // constructors
    vec(){first = end = last = nullptr;}
    explicit vec(size_t size){first = new type[size](); end = first + size; last = end;}
    vec(size_t size, const type& val) 
    {
        first = new type[size];  
        for(size_t i=0; i<size; i++)
            first[i] = val; 
        end = first + size; 
        last = end;
    }

    void push_back( type data)
    {
        if(last == end)
            grow(); // code the grow function 
    
        *last = data;
        last++;
    }

    void pop_back()
    {
        if(last == first)
            return;
        last--;
        *last = type();
    }
    void erase(size_t index)
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

    // void erase(size_t from, size_t to)
    // {
    //     size_t s = size();
    //     if(to > s || from > to)
    //         throw std::out_of_range("Vector size was exceeded");

    //     for(size_t i = to; i<s; i++)
    //     {
    //         first[i-from] = std::move(first[i]);
    //     }
    //     for(size_t i =0; i < to-from; i++)
    //         pop_back();
    // }

    void insert(size_t pos, type val)
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

    void clear()
    {
        last = first;
    }

    size_t size() const
    {
        return last-first;
    }

    type& at(size_t index) 
    {
        if (index>= size())
             throw std::out_of_range("Vector size was exceeded");
        return first[index];
    }

    type& operator[](size_t index)
    {
        return first[index];
    }

    bool empty() const
    {
        return first == last;
    }
    type& front()
    {
        return *first;
    }
    type& back()
    {
        return *(last-1);
    }
    ~vec()
    {
        delete [] first;
    }


};

#endif
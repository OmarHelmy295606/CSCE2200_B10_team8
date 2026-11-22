#ifndef HASHMAP_HPP
#define HASHMAP_HPP
#include <string>

inline unsigned int hashOf(int key)  
{
    return static_cast<unsigned int>(key) * 2654435769u;
}

inline unsigned int hashOf(unsigned int key)  
{
    return (key) * 2654435769u;
}

inline unsigned int hashOf(const std::string& key)  
{
    unsigned int h = 2166136261u;
    for(char c: key)
    {
        h ^= static_cast<unsigned char>(c);
        h*= 16777619u;
    }
    return h;
}


template<typename K, typename V>
class hashMap 
{
    private:
        struct Node 
        {
            K key;
            V value;
            Node* next;

            Node(K k, V v, Node* n) : key(k), value(v), next(n) {}
        };
        Node** buckets;
        int capacity;
        int count;
        double maxLoadFactor;

        int indexFor(const K& key, int cap) const;
        Node* findNode(const K& key) const;
        void rehash(int newCapacity);

    public:
        hashMap();
        ~hashMap();
        hashMap(const hashMap&) = delete;  //copy constructor
        hashMap& operator=(const hashMap&) = delete; //copy assignment 

        void insert(const K& key, const V& value);
        V* find(const K& key);
        const V* find(const K& key) const;
        bool contains(const K& key) const;
        bool erase(const K& key);
        int size() const;
        bool isEmpty() const;
        void clear();
};

#include "../../src/hashMap.cpp"

#endif

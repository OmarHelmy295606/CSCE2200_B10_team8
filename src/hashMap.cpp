#include <iostream>
#include "../include/ds/hashMap.hpp"

template<typename K, typename V>
hashMap<K, V>::hashMap() : buckets(nullptr), capacity(53), count(0), maxLoadFactor(0.75)
{
    buckets = new Node*[capacity];
    for(int i=0;i<capacity;i++) buckets[i] = nullptr;
}

template<typename K, typename V> 
void hashMap<K,V>::clear()
{
    for(int i=0;i<capacity;i++)
    {
        Node* cur = buckets[i];
        while(cur != nullptr)
        {
            Node* nxt = cur->next;
            delete cur;
            cur = nxt;
        }
        buckets[i] = nullptr;
    }
    count = 0;
}

template<typename K, typename V> 
hashMap<K,V>::~hashMap()
{
    clear();
    delete[] buckets;
}

template<typename K, typename V>
int hashMap<K,V>::indexFor(const K& key, int cap) const
{
    return static_cast<int>(hashOf(key) % static_cast<unsigned int>(cap));
}

template<typename K, typename V>
typename hashMap<K,V>::Node* hashMap<K,V>::findNode(const K& key) const
{
    int index = indexFor(key, capacity);
    Node* cur = buckets[index];
    while(cur != nullptr)
    {
        if(cur -> key == key) return cur;
        cur = cur->next;
    }
    return nullptr;
}

template<typename K, typename V>
void hashMap<K,V>::rehash(int newcapacity)
{
    int primes[] = {53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 393241, 786433, 1572869};

    int target = newcapacity;
    for(int i=0;i<16;i++)
    {
        if(primes[i] >= target)
        {
            target = primes[i];
            break;
        }
    }
    Node** newBuckets = new Node*[target];
    for(int i=0;i<target;i++) newBuckets[i] = nullptr;

    for(int i=0;i<capacity;i++)
    {
        Node* cur = buckets[i];
        while(cur!=nullptr)
        {
            Node* nxt = cur->next;
            int idx = indexFor(cur->key, target);
            cur->next = newBuckets[idx];
            newBuckets[idx] = cur;
            cur = nxt;
        }
    }
    delete[] buckets;
    buckets = newBuckets;
    capacity = target;
}

template<typename K, typename V>
void hashMap<K,V>::insert(const K& key, const V& value)
{
    Node* existing = findNode(key);

    if(existing != nullptr)
    {
        existing->value = value;
        return;
    }

    if(static_cast<double>(count)/capacity > maxLoadFactor)
    {
        rehash(capacity*2);
    }

    int index = indexFor(key, capacity);
    buckets[index] = new Node(key, value, buckets[index]);
    count++;
}

template<typename K, typename V>
V* hashMap<K,V>::find(const K& key)
{
    Node* node = findNode(key);
    if (node == nullptr) return nullptr;
    return &node->value;
}

template<typename K, typename V>
const V* hashMap<K,V>::find(const K& key) const
{
    Node* node = findNode(key);
    if (node == nullptr) return nullptr;
    return &node->value;
}

template<typename K, typename V>
bool hashMap<K,V>::contains(const K& key) const 
{
    return findNode(key) != nullptr;
}

template<typename K, typename V>
int hashMap<K,V>::size() const 
{
    return count;
}

template<typename K, typename V>
bool hashMap<K,V>::erase(const K &key)
{
    int index = indexFor(key, capacity);
    Node* cur = buckets[index];
    Node* prev = nullptr;

    while (cur != nullptr) 
    {
        if (cur->key == key) 
        {
            if (prev == nullptr) buckets[index] = cur->next;
            else prev->next = cur->next;
            delete cur;
            count--;
            return true;
        }
        prev = cur;
        cur = cur->next;
    }
    return false;
}

template<typename K, typename V>
bool hashMap<K,V>::isEmpty() const { return count == 0; }
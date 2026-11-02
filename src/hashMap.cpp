#include <iostream>
#include "hashMap.hpp"

struct Node{
int key;
int value;
Node* next;

Node(int k, int v, Node* n) : key(k), value(v) , next(n) {}
};

class hashMap{
private:

Node** buckets;
int capacity;
int count;
double maxLoadFactor;

unsigned int hash(int key) const;
int indexFor(int key, int cap) const;
Node* findNode(int key) const;
void rehash(int newcapacity);

public:

hashMap();
~hashMap();

void insert(int key, int value);
int* find(int key);
bool contains(int key) const;
bool erase(int key);
int size() const;
bool isEmpty() const;

};

hashMap::hashMap() : buckets(nullptr), capacity(53), count(0), maxLoadFactor(0.75){
buckets = new Node*[capacity];
for(int i=0;i<capacity;i++) buckets[i] = nullptr;
}

hashMap::~hashMap(){
for(int i=0;i<capacity;i++){
Node* cur = buckets[i];
while(cur != nullptr){
Node* nxt = cur->next;
delete cur;
cur = nxt;
}
}
delete[] buckets;
}


unsigned int hashMap::hash(int key) const{
return static_cast<unsigned int>(key) * 2654435769u;
}

int hashMap::indexFor(int key, int cap) const{
return static_cast<int>(hash(key) % static_cast<unsigned int>(cap));
}

Node* hashMap::findNode(int key) const{
int index = indexFor(key, capacity);
Node* cur = buckets[index];
while(cur != nullptr){
if(cur -> key == key) return cur;
cur = cur->next;
}
return nullptr;
}

void rehash(int newcapacity){
int primes[] = {53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 393241, 786433, 1572869};

int target = newcapacity;
for(int i=0;i<16;i++){
if(primes[i] >= target){
target = primes[i];
break;
}
}
Node** newBuckets = new Node*[target];
for(int i=0;i<target;i++) newBuckets[i] = nullptr;

for(int i=0;i<capacity;i++){
Node* cur = buckets[i];
while(cur!=nullptr){
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

void insert(int key, int value){
Node* existing = findNode(key);

if(existing != nullptr){
existing->value = value;
return;
}

if(static_cast<double>(count)/capacity > maxLoadFactor){
rehash(capacity*2);
}

int index = indexFor(key, capacity);
buckets[index] = Node(key, value, buckets[index]);
count++;

}

int* hashMap::find(int key){
Node* node = findNode(key);
if (node == nullptr) return nullptr;
return node;
}

bool hashMap::contains(int key) const {
return findNode(key) != nullptr;
}

int hashMap::size() const {
return count;
}

bool hashMap::erase(int key) {
    int index = indexFor(key, capacity);
    Node* cur = buckets[index];
    Node* prev = nullptr;

    while (cur != nullptr) {
        if (cur->key == key) {
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

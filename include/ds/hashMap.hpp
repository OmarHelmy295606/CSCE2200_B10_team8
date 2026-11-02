#ifndef HASHMAP_HPP
#define HASHMAP_HPP

struct Node {
int key;
int value;
Node* next;

Node(int k, int v, Node* n) : key(k), value(v), next(n) {}
};

class hashMap {
private:
Node** buckets;
int capacity;
int count;
double maxLoadFactor;

unsigned int hash(int key) const;
int indexFor(int key, int cap) const;
Node* findNode(int key) const;
void rehash(int newCapacity);

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

#endif

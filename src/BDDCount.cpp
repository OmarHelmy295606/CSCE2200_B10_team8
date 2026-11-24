#include "../include/BDDCount.h"

class BDDCountHelper {
private:
    const BDDStorage& store;
    int numVars;
    hashMap<int, unsigned long long> memo;

    // count assignments starting at this node's own variable
    unsigned long long countNode(int node) {
        unsigned long long* saved = memo.find(node);
        if (saved)
            return *saved;

        int variable = store.getVar(node);
        int nextVariable = variable + 1;

        unsigned long long lowCount =
            countBranch(store.getLow(node), nextVariable);
        unsigned long long highCount =
            countBranch(store.getHigh(node), nextVariable);
        unsigned long long total = lowCount + highCount;

        memo.insert(node, total);
        return total;
    }

    // count the assignments represented by one outgoing branch
    unsigned long long countBranch(int child, int nextVariable) {
        if (store.isTerminal(child)) {
            if (child == 0)
                return 0;

            // terminal 1 accepts every setting of the remaining variables
            return 1ULL << (numVars - nextVariable);
        }

        int childVariable = store.getVar(child);
        int skippedVariables = childVariable - nextVariable;

        // each skipped variable can be either 0 or 1, so each doubles the count
        return (1ULL << skippedVariables) * countNode(child);
    }

public:
    explicit BDDCountHelper(const BDDStorage& bdd)
        : store(bdd), numVars(bdd.numVars()) {}

    unsigned long long count(int root) {
        if (store.isTerminal(root))
            return root == 0 ? 0 : (1ULL << numVars);

        int rootVariable = store.getVar(root);

        // variables before the root are absent from the reduced BDD
        return (1ULL << rootVariable) * countNode(root);
    }
};

unsigned long long satCount(const BDDStorage& store, int root) {
    BDDCountHelper counter(store);
    return counter.count(root);
}
#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include "ds_Bitset.h"
#include "ds/vec.hpp"
#include "ds/hashMap.hpp"
#include "ds/heap.hpp"
// #include "Expression.h"
//The BFM class takes a boolean function and minimizes it using QM.

// used to know if an implicant was generated before
struct ImplicantKey{
    int value;
    int mask;
};
bool operator==(const ImplicantKey& a, const ImplicantKey& b);
unsigned int hashOf(const ImplicantKey& k);

class BFM{
    private:
    int variables_num = 0;
    vec<std::string> variable_names; // A, B, C or the names used in the expression

        struct implicant{
            // The mask is 1 when there is a dash, 0 if there isn't.
            // In the bits where there is a mask, the bit in value is a 0.
            // In any other bit in value, it is the true value of the bit (0/1)
            // 10-1 is value: 1001 & mask: 0010
        int value;
        int mask; // the dash places
        bool used;
        // the minterms it covers (in order, without the dont cares). Filled only when it becomes a prime implicant
        vec<int> covered;
    };

    // a prime implicant waiting in the heap of the greedy cover
    struct coverCandidate{
        int gain;     // the number of uncovered minterms it covers
        int literals;
        int index;    // its place in prime_implicants
        bool operator>(const coverCandidate& other) const;
    };

    vec<int> input_terms;
    vec<int> minterms;
    Bitset minterms_set;   //for quick lookup of minterms
    vec<int> dont_cares;
    Bitset dont_cares_set;

    vec<implicant> prime_implicants;
    vec<implicant> Essential_prime_implicants;
    Bitset is_essential;   // bit i is 1 if prime_implicants[i] is essential
    vec<implicant> Greedy_prime_implicants;
    vec<implicant> solution;
    std::string expression;


    bool is_using_minterms = true;

    std::string error_message = " ";

    implicant makeImplicant(int value, int mask);
    void addToGroup(hashMap<int, vec<implicant> >& groups, int ones, const implicant& x);
    void fillCovered(implicant& x);
    int countUncovered(const implicant& x, const Bitset& uncovered);
    void resetData();

    std::string expressionOf(const vec<implicant>& terms);

    void printTermList(const std::string label, const vec<int>& terms);
    void ePI_printer(std::string type, const vec<implicant>& terms);
    std::string termOf(const implicant& x);
    int literalsOf(const implicant& x);
    bool validInput(std::string inputfile); //Used in loadBF to validate the input
    int CountOnes(int bin_term);
    void removeExtraWindowsFileLetters(std::string& line);
    bool readNumberofVariables(std::string line);
    bool readTerms(std::string line, char letter, Bitset& is_row_used, vec<int>& terms);
    void convertMaxtermsToMinterms(Bitset& is_row_used);
    bool isMinterm(int k);


    public:
    //constructor
    BFM();
    std::string decimalToBinary(int n, int m);
    bool loadBF(std::string inputfile);
    // the rows that are 1 in dont_care_rows are dont cares even if they are 1 in table
// function to load the truth table
    void printInput();
    void generatePrimeImplicants();
    void generateEPIs();
    vec<int> getUncoveredMinterms();
    void printUncoveredMinterms();
    void greedyCover();
    void generateExpressions();
    void printExpression();

    // true if the solution covers every minterm and no row that is 0 in the function
    bool solutionIsCorrect();
    int primeImplicantsCount();
    int essentialPrimeImplicantsCount();
    int solutionSize();     // the number of prime implicants in the solution
    int solutionLiterals();
    std::string getExpression();
};

#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <string>
#include "ds/vec.hpp"
#include "ds/hashMap.hpp"
#include "ds_Bitset.h"

// keeps track of variable names and their indices (0, 1, 2, ...) in order of appearance
// var 0 is the MSB in truth-table rows and the top variable in the BDD
class VarTable {
private:
    vec<std::string> names; // index -> name
    hashMap<std::string, int> indexMap; // name -> index

public:
    int add(const std::string& name); // adds name if new, returns its index
    int indexOf(const std::string& name) const; // returns -1 if missing
    const std::string& name(int i) const;
    int size() const;
};

// token kinds produced by tokenize (postfix only keeps VAR, CONST, NOT, AND, XOR, OR)
enum TokenType {
    TOK_VAR,
    TOK_CONST,
    TOK_NOT,       // ! or ~
    TOK_NOT_AFTER, // '
    TOK_AND,       // *, &, ., or implicit AB
    TOK_XOR,       // ^
    TOK_OR,        // + or |
    TOK_LPAREN,
    TOK_RPAREN
};

struct Token {
    int type;
    int value; // variable index for TOK_VAR, 0/1 for TOK_CONST, 0 otherwise
};

class Expression {
private:
    vec<Token> postfixList; // saved postfix tokens after shunting-yard
    std::string errorMsg;   // error message if parse fails

    bool tokenize(const std::string& text, VarTable& vars, vec<Token>& tokens);
    bool toPostfix(const vec<Token>& tokens);

public:
    bool parse(const std::string& text, VarTable& vars);
    const std::string& error() const;
    const vec<Token>& postfix() const;

    bool evaluate(int row, int numVars) const; // evaluates one truth-table row
    Bitset truthTable(int numVars) const;      // evaluates all 2^numVars rows into a Bitset
};

// prints the truth table and minterms for an expression (to be called from main.cpp)
void runTable(const std::string& exprText, VarTable& vars);

#endif
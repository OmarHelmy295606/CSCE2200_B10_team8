#include "../include/Expression.h"
#include "../include/ds/stack.hpp"
#include <iostream>
#include <stdexcept>

int VarTable::add(const std::string& name) {
    int* existing = indexMap.find(name);
    if (existing != nullptr) {
        return *existing; // already in the table, return its old index
    }

    int idx = names.size(); // next free index
    names.push_back(name);
    indexMap.insert(name, idx);
    return idx;
}

int VarTable::indexOf(const std::string& name) const {
    const int* found = indexMap.find(name);
    if (found != nullptr) {
        return *found;
    }
    else {
        return -1; // not found
    }
}

const std::string& VarTable::name(int i) const {
    if (i < 0 || i >= size()) {
        throw std::out_of_range("Variable index is out of range!");
    }
    return names[i];
}

int VarTable::size() const {
    return names.size();
}

const std::string& Expression::error() const {
    return errorMsg;
}

const vec<Token>& Expression::postfix() const {
    return postfixList;
}

// small character helpers used inside tokenize
static bool isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static bool isSpace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

// checks if a token ends an operand (left side of an implicit AND, like A in AB or ' in A'B)
static bool endsOperand(int type) {
    return type == TOK_VAR || type == TOK_CONST ||
           type == TOK_RPAREN || type == TOK_NOT_AFTER;
}

// checks if a token starts a new operand (right side of an implicit AND, like B, (, or !)
static bool startsOperand(int type) {
    return type == TOK_VAR || type == TOK_CONST ||
           type == TOK_LPAREN || type == TOK_NOT;
}

// turns the expression string into tokens and adds implicit AND where operands touch
bool Expression::tokenize(const std::string& text, VarTable& vars, vec<Token>& tokens) {
    tokens.clear();
    errorMsg = "";

    size_t i = 0;
    while (i < text.size()) {
        char c = text[i];

        if (isSpace(c)) {
            i++;
            continue;
        }

        Token nextTok;

        if (isLetter(c)) { // variable: one letter + any digits after it (A0 is 1 var, x1y1 is 2 vars)
            std::string varName = "";
            varName += c;
            i++;

            while (i < text.size() && isDigit(text[i])) {
                varName += text[i];
                i++;
            }

            int id = vars.add(varName);
            nextTok = { TOK_VAR, id };
        }
        else if (c == '0' || c == '1') { // constant 0 or 1
            nextTok = { TOK_CONST, c - '0' };
            i++;
        }
        else if (c == '!' || c == '~') { // prefix NOT
            nextTok = { TOK_NOT, 0 };
            i++;
        }
        else if (c == '\'') { // postfix NOT
            nextTok = { TOK_NOT_AFTER, 0 };
            i++;
        }
        else if (c == '*' || c == '&' || c == '.') {
            nextTok = { TOK_AND, 0 };
            i++;
        }
        else if (c == '^') {
            nextTok = { TOK_XOR, 0 };
            i++;
        }
        else if (c == '+' || c == '|') {
            nextTok = { TOK_OR, 0 };
            i++;
        }
        else if (c == '(') {
            nextTok = { TOK_LPAREN, 0 };
            i++;
        }
        else if (c == ')') {
            nextTok = { TOK_RPAREN, 0 };
            i++;
        }
        else { // unknown char like # or 2
            errorMsg = "Unexpected character: ";
            errorMsg += c;
            return false;
        }

        // insert hidden AND when two operands touch (e.g. AB, A'C, x1y1, A(B+C))
        if (!tokens.empty() && endsOperand(tokens.back().type) && startsOperand(nextTok.type)) {
            tokens.push_back({ TOK_AND, 0 });
        }

        tokens.push_back(nextTok);
    }

    if (tokens.empty()) { // empty or whitespace-only input
        errorMsg = "Expression is empty!";
        return false;
    }

    return true;
}

// precedence: NOT > AND > XOR > OR
static int precedence(int type) {
    if (type == TOK_OR)  return 1;
    if (type == TOK_XOR) return 2;
    if (type == TOK_AND) return 3;
    if (type == TOK_NOT) return 4;
    return 0;
}

// shunting-yard: converts infix tokens into postfixList using opStack
bool Expression::toPostfix(const vec<Token>& tokens) {
    postfixList.clear();
    errorMsg = "";

    stack<Token> opStack;
    bool expectOperand = true; // true when we need a var, const, (, or ! next

    for (size_t i = 0; i < tokens.size(); i++) {
        Token tok = tokens[i];

        if (tok.type == TOK_VAR || tok.type == TOK_CONST) {
            if (!expectOperand) {
                errorMsg = "Unexpected operand!";
                return false;
            }
            postfixList.push_back(tok); // operands go straight to output
            expectOperand = false;
        }
        else if (tok.type == TOK_NOT) { // prefix ! or ~ waits on the stack for its operand
            if (!expectOperand) {
                errorMsg = "Unexpected '!' or '~' operator!";
                return false;
            }
            opStack.push(tok);
        }
        else if (tok.type == TOK_NOT_AFTER) { // postfix ' negates the operand we just read
            if (expectOperand) {
                errorMsg = "Postfix ' must come after an operand!";
                return false;
            }
            postfixList.push_back({ TOK_NOT, 0 }); // goes straight out as normal NOT
        }
        else if (tok.type == TOK_LPAREN) {
            if (!expectOperand) {
                errorMsg = "Unexpected '('!";
                return false;
            }
            opStack.push(tok);
        }
        else if (tok.type == TOK_RPAREN) {
            if (expectOperand) { // catches () or (A+)
                errorMsg = "Unexpected ')' or empty parentheses!";
                return false;
            }

            // pop everything until the matching '('
            bool foundLeftParen = false;
            while (!opStack.empty()) {
                Token topOp = opStack.pop();
                if (topOp.type == TOK_LPAREN) {
                    foundLeftParen = true;
                    break;
                }
                postfixList.push_back(topOp);
            }

            if (!foundLeftParen) {
                errorMsg = "Missing opening '('!";
                return false;
            }
        }
        else if (tok.type == TOK_AND || tok.type == TOK_XOR || tok.type == TOK_OR) {
            if (expectOperand) { // catches +A or A+*B
                errorMsg = "Missing operand before binary operator!";
                return false;
            }

            // pop operators on the stack that have higher or equal precedence first
            while (!opStack.empty() && opStack.top().type != TOK_LPAREN &&
                   precedence(opStack.top().type) >= precedence(tok.type)) {
                postfixList.push_back(opStack.pop());
            }

            opStack.push(tok);
            expectOperand = true; // need the right operand next
        }
    }

    if (expectOperand) { // catches trailing operators like A+ or !
        errorMsg = "Expression ended unexpectedly (missing operand)!";
        return false;
    }

    // flush the remaining operators on the stack
    while (!opStack.empty()) {
        Token topOp = opStack.pop();
        if (topOp.type == TOK_LPAREN) {
            errorMsg = "Missing closing ')'!";
            return false;
        }
        postfixList.push_back(topOp);
    }

    return true;
}

bool Expression::parse(const std::string& text, VarTable& vars) {
    vec<Token> tokens;
    if (!tokenize(text, vars, tokens)) {
        postfixList.clear();
        return false;
    }
    if (!toPostfix(tokens)) {
        postfixList.clear();
        return false;
    }
    return true;
}

// evaluates postfixList for one row of the truth table using stack<bool>
bool Expression::evaluate(int row, int numVars) const {
    if (postfixList.empty()) {
        throw std::runtime_error("Expression is empty or was not parsed!");
    }

    stack<bool> values;

    for (size_t i = 0; i < postfixList.size(); i++) {
        Token tok = postfixList[i];

        if (tok.type == TOK_VAR) { // read variable i's bit from row (var 0 is the MSB)
            if (tok.value < 0 || tok.value >= numVars) {
                throw std::out_of_range("Variable index is out of range for numVars!");
            }
            int shift = numVars - 1 - tok.value; // steps from the right edge of row
            bool bit = ((row >> shift) & 1) != 0;
            values.push(bit);
        }
        else if (tok.type == TOK_CONST) { // push constant 0 (false) or 1 (true)
            values.push(tok.value != 0);
        }
        else if (tok.type == TOK_NOT) { // pop top bool, flip it, and push back
            bool a = values.pop();
            values.push(!a);
        }
        else if (tok.type == TOK_AND) { // pop two bools (b is on top, a below) and push a && b
            bool b = values.pop();
            bool a = values.pop();
            values.push(a && b);
        }
        else if (tok.type == TOK_XOR) { // pop two bools and push a != b (1 when they differ)
            bool b = values.pop();
            bool a = values.pop();
            values.push(a != b);
        }
        else if (tok.type == TOK_OR) { // pop two bools and push a || b
            bool b = values.pop();
            bool a = values.pop();
            values.push(a || b);
        }
    }

    return values.pop(); // final result for this row
}

// evaluates all 2^numVars rows and marks minterms (rows giving 1) in a Bitset
Bitset Expression::truthTable(int numVars) const {
    if (numVars < 0 || numVars > 20) {
        throw std::out_of_range("numVars out of range for truth table!");
    }

    int numRows = 1 << numVars; // 2^numVars
    Bitset table(numRows);

    for (int row = 0; row < numRows; row++) {
        if (evaluate(row, numVars)) {
            table.set(row);
        }
    }

    return table;
}

// prints the truth table and minterms for an expression (to be called from main.cpp)
void runTable(const std::string& exprText, VarTable& vars) {
    Expression expr;
    if (!expr.parse(exprText, vars)) {
        std::cout << "Parse error: " << expr.error() << std::endl;
        return;
    }

    int n = vars.size();
    Bitset table = expr.truthTable(n);
    int numRows = table.size();

    // header row: variable names | F
    for (int i = 0; i < n; i++) {
        std::cout << vars.name(i) << " ";
    }
    std::cout << "| F" << std::endl;

    // separator line matching each variable name's width
    for (int i = 0; i < n; i++) {
        for (size_t k = 0; k < vars.name(i).size(); k++) {
            std::cout << "-";
        }
        std::cout << "-";
    }
    std::cout << "+--" << std::endl;

    // print every row from 0 to 2^n - 1
    for (int row = 0; row < numRows; row++) {
        for (int i = 0; i < n; i++) {
            int bit = (row >> (n - 1 - i)) & 1; // var 0 is MSB
            for (size_t k = 1; k < vars.name(i).size(); k++) {
                std::cout << " "; // align multi-char names like x1
            }
            std::cout << bit << " ";
        }

        if (table.test(row)) {
            std::cout << "| 1" << std::endl;
        }
        else {
            std::cout << "| 0" << std::endl;
        }
    }

    // print minterms list
    std::cout << "Minterms: m(";
    bool first = true;
    for (int row = 0; row < numRows; row++) {
        if (table.test(row)) {
            if (!first) { // add comma before every minterm except the first one
                std::cout << ", ";
            }
            std::cout << row;
            first = false;
        }
    }
    std::cout << ")" << std::endl;
}
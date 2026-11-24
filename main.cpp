#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include "include/Expression.h"
#include "include/BooleanFunctionMinimizer.h"
#include "include/BDDEngine.h"

const int MAX_Variables = 20;
const int MAX_BDD_VARIABLES = 30;

void printUsage()
{
    std::cout << "Usage:\n";
    std::cout << "  bfm table \"<expression>\"                      print the truth table\n";
    std::cout << "  bfm minimize \"<expression>\"                   minimize with Quine-McCluskey\n";
    std::cout << "  bfm minimize \"<expression>\" -d \"<dont cares>\" the rows where the second expression is 1 are dont cares\n";
    std::cout << "  bfm minimize -f <input-file>                  minimize a file of minterms (m), maxterms (M), dont cares (d)\n";
    std::cout << "  bfm count \"<expression>\"                      count satisfying assignments\n";
    std::cout << "  bfm bdd \"<expression>\" [-dot <file>]          show BDD statistics and optionally save the diagram\n";
    std::cout << "\nExpression: variables are a letter with optional digits (A, x1).\n";
    std::cout << "  NOT: !A or A'    AND: AB or A*B    XOR: A^B    OR: A+B\n";
}

bool loadExpression(const std::string &text, VarTable &vars, Expression &expression, int max_variables = MAX_Variables)
{
    if (!expression.parse(text, vars))
    {
        std::cerr << "ERROR: " << expression.error() << std::endl;
        return false;
    }
    if (vars.size() > max_variables)
    {
        std::cerr << "ERROR: number of variables must be at most " << max_variables << std::endl;
        return false;
    }
    return true;
}

void minimizeAndPrint(BFM &function)
{
    function.generatePrimeImplicants();
    function.generateEPIs();
    function.printInput();
    function.greedyCover();
    function.generateExpressions();
    function.printExpression();
}

int runTableCommand(const std::string &text)
{
    // parse once here to check the number of variables before the 2^n rows are made
    VarTable check_vars;
    Expression check;
    if (!loadExpression(text, check_vars, check))
        return 1;

    // runTable is written by Member 3 in Expression.cpp
    VarTable vars;
    runTable(text, vars);
    return 0;
}

int runCount(const std::string &text)
{
    VarTable vars;
    Expression expression;

    if (!loadExpression(text, vars, expression, MAX_BDD_VARIABLES))
        return 1;

    BDDEngine engine(vars.size());
    int root = engine.fromExpression(expression);
    std::cout << engine.satCount(root) << " of " << (1ULL << vars.size()) << '\n';
    return 0;
}

int runBDD(const std::string &text, const char *dotFile = nullptr)
{
    VarTable vars;
    Expression expression;

    if (!loadExpression(text, vars, expression, MAX_BDD_VARIABLES))
        return 1;

    BDDEngine engine(vars.size());
    int root = engine.fromExpression(expression);

    std::cout << "Reachable nodes: " << engine.storage().countReachable(root) << '\n';
    std::cout << "Stored nodes: " << engine.storage().size() << '\n';
    std::cout << "Cache hits: " << engine.cacheHits()
              << ", misses: " << engine.cacheMisses() << '\n';

    if (dotFile == nullptr)
        return 0;

    std::ofstream out(dotFile);
    if (!out)
    {
        std::cerr << "ERROR: could not open " << dotFile << '\n';
        return 1;
    }

    engine.writeDot(root, vars, out);

    out.close();
    if (!out)
    {
        std::cerr << "ERROR: could not finish writing " << dotFile << '\n';
        return 1;
    }
    return 0;
}

int runMinimize(const std::string &text, const std::string &dont_cares)
{
    // the two expressions share the variable table, so a name means the same variable in both
    VarTable vars;
    Expression expression, dont_care_expression;
    if (!loadExpression(text, vars, expression))
        return 1;
    if (!dont_cares.empty() && !loadExpression(dont_cares, vars, dont_care_expression))
        return 1;

    int n = vars.size();
    Bitset table = expression.truthTable(n);
    Bitset dont_care_rows(table.size());
    if (!dont_cares.empty())
        dont_care_rows = dont_care_expression.truthTable(n);

    // the minimizer takes the names as a list, so it does not depend on the parser
    vec<std::string> names;
    for (int i = 0; i < n; i++)
        names.push_back(vars.name(i));

    BFM function;
    function.loadTruthTable(table, dont_care_rows, names);
    minimizeAndPrint(function);
    return 0;
}

int runMinimizeFile(const std::string &inputfile)
{
    BFM function;
    if (!function.loadBF(inputfile))
    {
        return 1; // the error is printed already
    }
    minimizeAndPrint(function);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printUsage();
        return 1;
    }
    std::string command = argv[1];

    try
    {
        if (command == "table" && argc == 3)
            return runTableCommand(argv[2]);

        if (command == "count" && argc == 3)
            return runCount(argv[2]);

        if (command == "bdd" && argc == 3)
            return runBDD(argv[2]);

        if (command == "bdd" && argc == 5 && std::string(argv[3]) == "-dot")
            return runBDD(argv[2], argv[4]);

        if (command == "minimize" && argc == 4 && std::string(argv[2]) == "-f")
            return runMinimizeFile(argv[3]);

        if (command == "minimize" && argc == 3)
            return runMinimize(argv[2], "");

        if (command == "minimize" && argc == 5 && std::string(argv[3]) == "-d")
            return runMinimize(argv[2], argv[4]);
    }
    catch (const std::exception &e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    printUsage();
    return 1;
}
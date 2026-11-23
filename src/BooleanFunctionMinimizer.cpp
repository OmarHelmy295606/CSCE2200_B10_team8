#include "../include/BooleanFunctionMinimizer.h"

bool operator==(const ImplicantKey& a, const ImplicantKey& b){
    return a.value == b.value && a.mask == b.mask;
}

unsigned int hashOf(const ImplicantKey& k){
    return (unsigned int)k.value * 1000003u + (unsigned int)k.mask;
}

BFM::BFM() : minterms_set(1), dont_cares_set(1), is_essential(1) {}

void BFM::resetData(){
    int rows = 1 << variables_num;
    input_terms.clear();
    minterms.clear();
    dont_cares.clear();
    minterms_set = Bitset(rows);
    dont_cares_set = Bitset(rows);
    prime_implicants.clear();
    Essential_prime_implicants.clear();
    Greedy_prime_implicants.clear();
    solution.clear();
    expression.clear();
}

bool BFM::validInput(std::string inputfile){
    std::ifstream input(inputfile);

    //check if input is opened
    if (!input) {
        error_message = "ERROR: could not open the file!" ;
        return false;
    }

    //read file word by word
    std::string line1, line2, line3; //if there are extra lines we will ignore them.


    //checking if there are any empty lines.
    if(!std::getline(input, line1) || !std::getline(input, line2)){
        error_message = "ERROR: Missing lines from input file!";
        return false;
    }
    std::getline(input, line3); // No third line means there are no DCs.

    input.close();

    removeExtraWindowsFileLetters(line1);
    removeExtraWindowsFileLetters(line2);
    removeExtraWindowsFileLetters(line3);


    if(line1.empty() || line2.empty()){
        error_message = "ERROR: Missing lines from input file!";
        return false;
    }

    //check line 1 (number of variables)

    if(!readNumberofVariables(line1)){
        return false;
    }
    resetData();
    int number_of_rows = 1 << variables_num;

    Bitset is_row_used(number_of_rows);

    if(line2[0] == 'm'){
        is_using_minterms = true;
        if(!readTerms(line2, 'm', is_row_used, input_terms)) return false;
    }

    else if(line2[0] == 'M'){
        is_using_minterms = false;
        if(!readTerms(line2, 'M', is_row_used, input_terms)) return false;
    }
    else{
        error_message = "ERROR: line 2 must start with 'm' (minterms) or 'M' (maxterms)";
        return false;
    }

    if(!readTerms(line3, 'd', is_row_used, dont_cares)) return false;
    for(size_t i=0;i<dont_cares.size();i++) dont_cares_set.set(dont_cares[i]);

    if(is_using_minterms){
        minterms = input_terms;
        for(size_t i=0;i<minterms.size();i++) minterms_set.set(minterms[i]);
    }
    else{
        convertMaxtermsToMinterms(is_row_used);
    }

    if(minterms.empty()){
        error_message = "ERROR: the function has no minterms";
        return false;
    }

    // the variables of a file are named A, B, C, ...
    variable_names.clear();
    for(int i=0;i<variables_num;i++){
        variable_names.push_back(std::string(1, char('A' + i)));
    }

    return true;
}

void BFM::removeExtraWindowsFileLetters(std::string& line){
    if(!line.empty() && line.back() == '\r')
        line.pop_back();
}

bool BFM::readNumberofVariables(std::string line){
    int n = 0;
    size_t line_size = line.size();
    for(size_t i = 0; i < line_size; i++){
        if (line[i] < '0' || line[i] > '9') {
             error_message = "ERROR: invalid number of variables";
             return false;
        }
        n = n * 10 + (line[i] - '0');
        if (n > 20) {
            error_message = "ERROR: number of variables must be between 1 and 20";
            return false;
        }
    }
    if(n < 1 || n > 20){
        error_message = "ERROR: number of variables must be between 1 and 20";
        return false;
    }

    variables_num = n;
    return true;
}

bool BFM::readTerms(std::string line, char letter, Bitset& is_row_used, vec<int>& terms){
    int max_number_of_rows = is_row_used.size() - 1;
    size_t line_size = line.size();
    std::string term_name;
    if(letter == 'm')   term_name = "minterm";
    else if(letter == 'M') term_name = "maxterm";
    else term_name = "dont care";

    size_t j =0;
    for(size_t i=0; i <line_size; i=j){
        if(line[i] != letter){
            if((letter == 'm' && line[i] == 'M') || (letter == 'M' && line[i] == 'm')){
                error_message = "ERROR: cannot mix minterms (m) and maxterms (M)";
            }
            else{
                error_message = "ERROR: invalid " + term_name + " (expected '" + letter + "')";
            }
            return false;
        }
        j = i+1;
        int n=0;
        while(j < line_size && line[j] >= '0' && line[j] <= '9'){
            n = n * 10 + (line[j] - '0');
            j++;
            if(n > max_number_of_rows) break; // preventing overflow
        }
        if(j == i + 1){
            error_message = "ERROR: missing number after '" + std::string(1, letter) + "'";
            return false;
        }
        // check if out of bounds
        if (n > max_number_of_rows) {
            error_message = "ERROR: invalid " + term_name + " (out of range)";
            return false;
        }

        //check if used before (on this line, or on line 2 for dont cares)
        if(is_row_used.test(n)){
            error_message = "ERROR: " + term_name + " " + std::to_string(n) + " used before";
            return false;
        }

        //update used rows and save the term
        is_row_used.set(n);
        terms.push_back(n);

        //check if there are still terms (i.e. if a comma is expected)
        if(j < line_size){

            //check if it is comma
            if(line[j] != ','){
                error_message = "ERROR: expected ',' between " + term_name + "s";
                return false;
            }

            //go to the next index
            j++;

            if(j == line_size){
                error_message = "ERROR: line ends with a comma";
                return false;
            }
        }
    }
    return true;
}

void BFM::convertMaxtermsToMinterms(Bitset& is_row_used){
    int number_of_rows = is_row_used.size();
    for(int row=0;row<number_of_rows;row++){
        if(!is_row_used.test(row)){
            minterms.push_back(row);
            minterms_set.set(row);
        }
    }
}

bool BFM::loadBF(std::string inputfile){
    if(!validInput(inputfile)){
        std::cerr << error_message << std::endl;
        return false;
    }

    return true;
}

// loading from truth table

void BFM::printTermList(const std::string label, const vec<int>& terms) {
    std::cout << label << ": ";

    if (terms.empty()) {
        std::cout << "none\n";
        return;
    }

    for (size_t i = 0; i < terms.size(); ++i) {
        if (i > 0) {
            std::cout << " ";
        }
        std::cout << terms[i];
    }
    std::cout << "\n";
}

//print prime implicants & essential prime implicants
void BFM::ePI_printer(std::string type, const vec<implicant>& terms) {
    std::cout << type<< ": \n";
    if(terms.empty()) std::cout << "none\n";

    // the width of a literal's column, so the names with digits (x1) stay aligned
    size_t cell_width = 3;
    for(int v=0;v<variables_num;v++){
        if(variable_names[v].size() + 2 > cell_width) cell_width = variable_names[v].size() + 2;
    }

    size_t size = terms.size();
    for(size_t i = 0; i < size; i++) {
        std::cout << '\n' << i+1 << " ";
        //loop over the variables of the prime implicant

        for(int v=0;v<variables_num;v++){
            int bit = variables_num - 1 - v;
            if((terms[i].mask >> bit) & 1) std::cout<<'-';
            else if((terms[i].value >> bit) & 1) std::cout<<'1';
            else std::cout<<'0';
        }
        std::cout << "   ";

        for(int v=0;v<variables_num;v++){
            int bit = variables_num - 1 - v;
            std::string cell = "-";

            if(!((terms[i].mask >> bit) & 1)){
                cell = variable_names[v];
                if(!((terms[i].value >> bit) & 1)) cell += "'";
            }
            std::cout << std::left << std::setw(cell_width) << cell;
        }
        std::cout << "      ";

        //print min & dont care terms
        std::cout << "minterms covered: ";
        for(size_t c=0;c<terms[i].covered.size();c++){
            std::cout << terms[i].covered[c] << " ";
        }
        std::cout << "\n" << std::setw(std::to_string(i+1).size() + 1 + variables_num + 3 + cell_width*variables_num + 6) << "" << "don't cares covered: ";
        // the dont cares are not stored in covered, so we go over the rows of the implicant again
        int sub = 0;
        while(true){
            if(dont_cares_set.test(terms[i].value | sub)) std::cout << (terms[i].value | sub) << " ";
            if(sub == terms[i].mask) break;
            sub = (sub - terms[i].mask) & terms[i].mask;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void BFM::printInput() {
    //print number of variables
    std::cout << "Number of Variables: " << variables_num << std::endl;
    if(!is_using_minterms){
        std::cout << "(the input was maxterms, converted to minterms)" << std::endl;
    }

    printTermList("minterms", minterms);
    printTermList("dontcares", dont_cares);

    std::cout << std::endl;

    //print prime implicants
    ePI_printer("Prime Implicants", prime_implicants);
    ePI_printer("Essential Prime Implicants", Essential_prime_implicants);
}

std::string BFM::decimalToBinary(int num, int n) {
    std::string bits = "";

    while (num > 0) {
        bits = char('0' + (num % 2)) + bits;
        num /= 2;
    }

    while ((int)bits.length() < n) {
        bits = "0" + bits;
    }

    return bits;
}

int BFM::CountOnes(int bin_term) {//binary input expected
    int ones=0;
    while(bin_term){
        bin_term = bin_term & (bin_term -1);
        ones++;
    }
    return ones;
}

BFM::implicant BFM::makeImplicant(int value, int mask){
    implicant x;
    x.value = value;
    x.mask = mask;
    x.used = false;
    return x;
}

// adds the implicant to the bucket of its number of ones, and makes the bucket if it is the first one
void BFM::addToGroup(hashMap<int, vec<implicant> >& groups, int ones, const implicant& x){
    vec<implicant>* group = groups.find(ones);
    if(group == nullptr){
        groups.insert(ones, vec<implicant>());
        group = groups.find(ones);
    }
    group->push_back(x);
}

// An implicant covers the rows we get by putting every 0/1 combination in its dashes.
// Going over the subsets of the mask in increasing order gives the rows in order.
void BFM::fillCovered(implicant& x){
    x.covered.clear();
    int sub = 0;
    while(true){
        if(isMinterm(x.value | sub)) x.covered.push_back(x.value | sub);
        if(sub == x.mask) break;
        sub = (sub - x.mask) & x.mask; // the next subset of the mask
    }
}

void BFM::generatePrimeImplicants(){
    prime_implicants.clear();
    // The hash map buckets group the terms by their number of ones (key: number of ones, value: the terms that have it)
    hashMap<int, vec<implicant> >* current = new hashMap<int, vec<implicant> >();
    for(size_t i=0;i<minterms.size();i++){
        int m = minterms[i];
        addToGroup(*current, CountOnes(m), makeImplicant(m, 0));
    }
    for(size_t i=0;i<dont_cares.size();i++){
        int d = dont_cares[i];
        addToGroup(*current, CountOnes(d), makeImplicant(d, 0));
    }

    hashMap<ImplicantKey, bool> implacants_added;
    bool something_to_merge = true;
    while(something_to_merge){
        something_to_merge = false; // if there is something to merge, the boolean will change the value inside the loop conditions
        hashMap<int, vec<implicant> >* next = new hashMap<int, vec<implicant> >();
        for(int k=0;k< variables_num; k++){
            vec<implicant>* group_a = current->find(k);
            vec<implicant>* group_b = current->find(k+1);
            //if this bucket or the next one is empty, go to the next one
            if(group_a == nullptr || group_b == nullptr) continue;
            for(size_t i=0;i<group_a->size();i++){
                implicant& a = (*group_a)[i];
                for(size_t j=0;j<group_b->size();j++){
                    implicant& b = (*group_b)[j];
                    if(a.mask != b.mask)   continue; // they need to have the same mask
                    int difference = a.value ^ b.value; // ^ is the XOR operator
                    if(difference ==0 || (difference & (difference - 1)) != 0){
                        continue;
                    } // they need to differ in exactly 1 bit
                    // if the two implicants passed this level, then they will be merged
                    a.used = true;
                    b.used = true;
                    something_to_merge = true;

                    //if the pair does not exist, add it to the 'next' buckets
                    ImplicantKey key = {a.value & b.value, a.mask | difference};
                    if(implacants_added.contains(key)) continue;
                    implacants_added.insert(key, true);

                    // the merged implicant lost a one from b, so it has k ones like a
                    addToGroup(*next, k, makeImplicant(key.value, key.mask));
                }
            }
        }

        // anything not covered in this round is a Prime Implicant
        for(int i=0;i<=variables_num;i++){
            vec<implicant>* group = current->find(i);
            if(group == nullptr) continue;
            for(size_t j=0;j<group->size();j++){
                implicant& x = (*group)[j];
                if(!x.used) {
                    //check if it covers any minterms, if not ignore it
                    fillCovered(x);
                    if(!x.covered.empty()) prime_implicants.push_back(x);
                }
            }
        }
        delete current;
        current = next;
    }
    delete current;
}


bool BFM::isMinterm(int k){
    return minterms_set.test(k);
}


void BFM::generateEPIs(){
    Essential_prime_implicants.clear();
    is_essential = Bitset((int)prime_implicants.size());

    // for every row: how many PIs cover it, and one of them (if there is only one, it is the essential one)
    int rows = 1 << variables_num;
    vec<int> cover_count(rows, 0);
    vec<int> last_PI(rows, -1);
    for(size_t i=0;i<prime_implicants.size();i++){
        const implicant& x = prime_implicants[i];
        for(size_t c=0;c<x.covered.size();c++){
            cover_count[x.covered[c]]++;
            last_PI[x.covered[c]] = (int)i;
        }
    }
    // check for the indices
    for(size_t i=0;i<minterms.size();i++){
        int m = minterms[i];
        if(cover_count[m] == 1){
            if(!is_essential.test(last_PI[m])){
                Essential_prime_implicants.push_back(prime_implicants[last_PI[m]]);
                is_essential.set(last_PI[m]);
            }
        }
    }
}

vec<int> BFM::getUncoveredMinterms(){
    // mark every minterm that an EPI covers
    Bitset covered_rows(1 << variables_num);
    for(size_t i=0;i<Essential_prime_implicants.size();i++){
        const implicant& epi = Essential_prime_implicants[i];
        for(size_t c=0;c<epi.covered.size();c++) covered_rows.set(epi.covered[c]);
    }

    vec<int> notCoveredMinterms;
    for(size_t i=0;i<minterms.size();i++){
        if(!covered_rows.test(minterms[i])) notCoveredMinterms.push_back(minterms[i]);
    }
    return notCoveredMinterms;
}
void BFM::printUncoveredMinterms(){
    printTermList("Minterms not covered by EPIs", getUncoveredMinterms());
}

//after getting the essential prime implicants, we need to choose which prime implicants to include for the final expression.

// the heap gives the candidate that covers the most uncovered minterms, and on a tie the one with the fewest literals
bool BFM::coverCandidate::operator>(const coverCandidate& other) const{
    if(gain != other.gain) return gain > other.gain;
    if(literals != other.literals) return literals < other.literals;
    return index < other.index;
}

int BFM::countUncovered(const implicant& x, const Bitset& uncovered){
    int count = 0;
    for(size_t c=0;c<x.covered.size();c++){
        if(uncovered.test(x.covered[c])) count++;
    }
    return count;
}

void BFM::greedyCover(){
    /*
    The greedy choice: always take the PI that covers the most minterms that are still uncovered.
    All the non essential PIs wait in a max heap ordered by that number (the gain).
    The gain stored in the heap gets old when other PIs are chosen, but it can only go down.
    So when we pop a PI we count its gain again:
        - same as stored  -> no PI in the heap can beat it, so we choose it
        - smaller         -> we push it back with the new gain and pop again
    This way we do not rebuild the heap after every choice.
    */
    Greedy_prime_implicants.clear();

    Bitset uncovered(1 << variables_num);
    vec<int> remaining_minterms = getUncoveredMinterms();
    for(size_t i=0;i<remaining_minterms.size();i++) uncovered.set(remaining_minterms[i]);

    heap<coverCandidate> candidates;
    for(size_t i=0;i<prime_implicants.size();i++){
        if(is_essential.test((int)i)) continue;
        int gain = countUncovered(prime_implicants[i], uncovered);
        if(gain == 0) continue; // everything it covers is already covered by the EPIs
        coverCandidate candidate = {gain, literalsOf(prime_implicants[i]), (int)i};
        candidates.push(candidate);
    }

    while(uncovered.any() && !candidates.empty()){
        coverCandidate best = candidates.pop();
        const implicant& x = prime_implicants[best.index];

        int gain = countUncovered(x, uncovered);
        if(gain == 0) continue;
        if(gain < best.gain){
            best.gain = gain;
            candidates.push(best);
            continue;
        }

        Greedy_prime_implicants.push_back(x);
        for(size_t c=0;c<x.covered.size();c++) uncovered.reset(x.covered[c]);
    }

    solution = Essential_prime_implicants;
    for(size_t i=0;i<Greedy_prime_implicants.size();i++) solution.push_back(Greedy_prime_implicants[i]);
}

int BFM::literalsOf(const implicant& x){
    return variables_num - CountOnes(x.mask);
}

// the product term of an implicant, 10-1 is AB'D
std::string BFM::termOf(const implicant& x){
    std::string term;
    for(int v=0;v<variables_num;v++){
        int bit = variables_num - 1 - v;
        if((x.mask >> bit) & 1)
            continue;

        term += variable_names[v];

        if(!((x.value >> bit) & 1))
            term += "'";
    }
    if(term.empty())
        term = "1";
    return term;
}

std::string BFM::expressionOf(const vec<implicant>& terms){
    std::string current_expression;

    // Generate the SOP expression
    for(size_t j = 0; j < terms.size(); j++){

        if(j > 0)
            current_expression += " + ";

        current_expression += termOf(terms[j]);
    }

    // No terms
    if(current_expression.empty())
        current_expression = "0";
    return current_expression;
}

void BFM::generateExpressions(){
    // Clear previous expressions
    expression.clear();
    // the expression of the greedy cover
    expression = expressionOf(solution);

}

void BFM::printExpression(){
    printUncoveredMinterms();
    ePI_printer("\nPrime Implicants chosen by the greedy cover", Greedy_prime_implicants);
    std::cout << "Minimized expression: " << expression << std::endl;
    std::cout << "(" << solutionSize() << " terms, " << solutionLiterals() << " literals)" << std::endl;
}

bool BFM::solutionIsCorrect(){
    Bitset covered_rows(1 << variables_num);
    for(size_t i=0;i<solution.size();i++){
        int sub = 0;
        while(true){
            int row = solution[i].value | sub;
            // an implicant can only have minterms and dont cares inside it
            if(!isMinterm(row) && !dont_cares_set.test(row)) return false;
            covered_rows.set(row);
            if(sub == solution[i].mask) break;
            sub = (sub - solution[i].mask) & solution[i].mask;
        }
    }
    return minterms_set.isSubsetOf(covered_rows);
}

int BFM::primeImplicantsCount(){
    return (int)prime_implicants.size();
}

int BFM::essentialPrimeImplicantsCount(){
    return (int)Essential_prime_implicants.size();
}

int BFM::solutionSize(){
    return (int)solution.size();
}

int BFM::solutionLiterals(){
    int literals = 0;
    for(size_t i=0;i<solution.size();i++) literals += literalsOf(solution[i]);
    return literals;
}

std::string BFM::getExpression(){
    return expression;
}
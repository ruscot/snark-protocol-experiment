#include "r1cs_evaluation.hpp"

/**
 * @brief Evaluate a R1CS cosntraint with the current variable assignment
 * Recall an R1CS constraint is as follow 
 * A * B = C
 * @tparam FieldT Field for the constraint
 * @param a Our A
 * @param b Our B
 * @param assignment 
 * @return FieldT corresponding to the value of C 
 */
template<typename FieldT>
FieldT evaluation_on_linear_combination(linear_combination<FieldT> a, linear_combination<FieldT> b, std::vector<FieldT> &assignment) {
    FieldT elemA = a.evaluate(assignment);
    FieldT elemB = b.evaluate(assignment);
    FieldT elemC = elemA * elemB;
    return elemC;
}


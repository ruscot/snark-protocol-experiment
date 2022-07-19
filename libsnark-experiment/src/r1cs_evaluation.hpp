#ifndef R1CS_EVALUATION_H
#define R1CS_EVALUATION_H

#include "libsnark/relations/variable.hpp"

#include "libff/algebra/fields/field_utils.hpp"
#include "libsnark/zk_proof_systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark.hpp"
#include "libsnark/common/default_types/r1cs_ppzksnark_pp.hpp"
#include "libsnark/gadgetlib1/pb_variable.hpp"

#include "libsnark/gadgetlib2/variable.hpp"

using namespace std;
using namespace libsnark;

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
FieldT evaluation_on_linear_combination(linear_combination<FieldT> a, linear_combination<FieldT> b, std::vector<FieldT> &assignment) ; 
#include "r1cs_evaluation.cpp"
#endif
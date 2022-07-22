#ifndef SNARK_POLYNOMIAL_IN_CLEAR_HPP
#define SNARK_POLYNOMIAL_IN_CLEAR_HPP

#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <time.h>

#include "libff/algebra/fields/field_utils.hpp"
#include "libsnark/zk_proof_systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark.hpp"
#include "libsnark/common/default_types/r1cs_ppzksnark_pp.hpp"
#include "libsnark/gadgetlib1/pb_variable.hpp"

#include "libsnark/gadgetlib2/variable.hpp"

#include "util.hpp"
#include "r1cs_from_poly_gen_function.hpp"
#include "r1cs_update_keys.hpp"

//#include "r1cs_evaluation.hpp"

#include <stdexcept>
#include <tuple>
#include <string> 

using namespace libsnark;
using namespace std;

/**
 * @brief compute the a_i of the R1CS
 * 
 * @tparam FieldT 
 * @tparam default_r1cs_ppzksnark_pp 
 * @param protoboard_for_poly 
 * @param pk 
 * @return r1cs_variable_assignment<FieldT> 
 */
template<typename FieldT, typename default_r1cs_ppzksnark_pp>
void compute_polynomial_witness_output(protoboard<FieldT> &protoboard_for_poly, 
    r1cs_ppzksnark_proving_key<default_r1cs_ppzksnark_pp> pk);

/**
 * @brief Simulate the update phase in our protocol
 * 
 * @tparam FieldT 
 * @param full_variable_assignment_update 
 * @param container_for_update 
 * @param last_keypair 
 * @param index_of_the_coef_to_update 
 * @param r1cs_polynomial_factory 
 * @return vector<double> return the different computing time during the update
 */
template<typename FieldT>
vector<double> test_polynomial_in_clear_update(r1cs_variable_assignment<FieldT> full_variable_assignment_update, 
                                return_container_key_generator_for_update<default_r1cs_ppzksnark_pp> container_for_update, 
                                r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> last_keypair, uint64_t index_of_the_coef_to_update,
                                R1CS_Polynomial_factory<FieldT, default_r1cs_ppzksnark_pp> r1cs_polynomial_factory);

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

/**
 * @brief This function will execute the different phases of our protocol on a polynomial
 * of degree polynomial_degree. We'll execute our protocol number_of_try time, to do a mean on 
 * the different timing of our protocol.
 * 
 * @param polynomial_degree 
 * @param number_of_try 
 */
void test_polynomial_in_clear(uint64_t polynomial_degree, int number_of_try);

void test_update_index_0(uint64_t polynomial_degree);

void test_update_index_1(uint64_t polynomial_degree);

void test_update_random_index(uint64_t polynomial_degree, int random_index_to_update);

//#include "snark_polynomial_in_clear.cpp"

#endif
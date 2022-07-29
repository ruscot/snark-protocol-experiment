/**
 * @file snark_tests.hpp
 * @author Martinez Anthony 
 * @brief Function to tests our protocol
 * @version 0.1
 * @date 2022-07-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef SNARK_TESTS_HPP
#define SNARK_TESTS_HPP

#include "snark_polynomial_in_clear.hpp"

using namespace libsnark;
using namespace std;

/**
 * @brief This function will test our protocol on a polynomial of degree 'polynomial_degree'. And we'll
 * try to update the coefficient 0 of our polynomial 2 times.
 * 
 * @param polynomial_degree 
 */
void test_update_index_0(uint64_t polynomial_degree);

/**
 * @brief This function will test our protocol on a polynomial of degree 'polynomial_degree'. And we'll
 * try to update the coefficient 1 of our polynomial 2 times.
 * 
 * @param polynomial_degree 
 */
void test_update_index_1(uint64_t polynomial_degree);

/**
 * @brief This function will test our protocol on a polynomial of degree 'polynomial_degree'. And we'll
 * try to update the coefficient 'random_index_to_update' of our polynomial 2 times.
 * 
 * @param polynomial_degree 
 */
void test_update_random_index(uint64_t polynomial_degree, int random_index_to_update);

/**
 * @brief This function will raised an error because the r1cs will not be generated correctly
 * on a polynomial of degree 'polynomial_degree'.
 * 
 * @param polynomial_degree 
 */
void test_raise_error_for_wrong_r1cs_construction(uint64_t polynomial_degree);

/**
 * @brief This function will raised an error because the r1cs will not be update correctly
 * on a polynomial of degree 'polynomial_degree' for the updated index 'random_index_to_update'
 * 
 * @param polynomial_degree 
 * @param random_index_to_update 
 */
void test_raise_error_for_wrong_r1cs_update(uint64_t polynomial_degree, int random_index_to_update);

/**
 * @brief This function will raised an error because the key will not be update correctly (if
 * the updated index 'random_index_to_update' > 1) on a polynomial of degree 'polynomial_degree' 
 * for the updated index 'random_index_to_update'.
 * 
 * @param polynomial_degree 
 * @param random_index_to_update 
 */
void test_raise_error_for_wrong_keys_update(uint64_t polynomial_degree, int random_index_to_update);

/**
 * @brief This function try to update a list of random coefficients 'random_index_to_update'.
 * We shall not have any errors.
 * 
 * @param polynomial_degree 
 * @param random_index_to_update 
 */
void test_update_multiple_index(uint64_t polynomial_degree, vector<int> random_index_to_update);

#endif
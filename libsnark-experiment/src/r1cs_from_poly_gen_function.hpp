#ifndef R1CS_FROM_POLY_GEN_FUNCTION_CPP
#define R1CS_FROM_POLY_GEN_FUNCTION_CPP


#include <tuple>
#include "r1cs_update_keys.hpp"
#ifndef CLOCKTYPE
#  ifdef CLOCK_PROCESS_CPUTIME_ID
/* cpu time in the current process */
#    define CLOCKTYPE  CLOCK_PROCESS_CPUTIME_ID
// #    define CLOCKTYPE  CLOCK_REALTIME
#  else
/* this one should be appropriate to avoid errors on multiprocessors systems */
#    define CLOCKTYPE  CLOCK_MONOTONIC
#  endif
#endif
#define VESPO_NANO_FACTOR 1.0e9
struct Chrono {
    struct timespec begin_time,end_time;
    void start() { clock_gettime(CLOCKTYPE, &begin_time); }
    double stop() {
	clock_gettime(CLOCKTYPE, &end_time);
	double ttime(difftime(end_time.tv_sec, begin_time.tv_sec));
	return ttime += ((double) (end_time.tv_nsec - begin_time.tv_nsec) )/ VESPO_NANO_FACTOR;
    }
};

/**
 * @brief This class will be used to generate a random polynomial and 
 * to perform the transformation to a R1CS and QAP for our protocol
 * You can perform the update and the generation of the keys with this
 * class
 * 
 * @tparam FieldT 
 * @tparam ppT 
 */
template<typename FieldT, typename ppT>
class R1CS_Polynomial_factory {
public:

    /**
     * @brief Construct a new r1cs polynomial factory object
     * 
     * @param degree the degree of the polynomial
     * @param insert_error_for_test we can insert some errors
     * in our protocol for the tests
     */
    R1CS_Polynomial_factory(uint64_t degree, int insert_error_for_test);

    /**
     * @brief Function to update the coefficient of index 0 of our polynomial
     * 
     * @param new_zero_coefficient_value 
     * @param last_zero_coefficient_value 
     */
    void update_coefficient_zero(libff::Fr<ppT> new_zero_coefficient_value, libff::Fr<ppT> last_zero_coefficient_value);

    /**
     * @brief Function to update the coefficient of index 1 of our polynomial
     * 
     * @param new_one_coefficient_value 
     * @param last_one_coefficient_value 
     */
    void update_coefficient_one(libff::Fr<ppT> new_one_coefficient_value, libff::Fr<ppT> last_one_coefficient_value);

    /**
     * @brief Set the value of the variable x which will be the evaluation point 
     * of our polynomial
     * 
     * @param x_value 
     */
    void set_x_value(libff::Fr<ppT> x_value);

    /**
     * @brief Get the degree of our polynomial
     * 
     * @return uint64_t 
     */
    uint64_t get_polynomial_degree();
    
    /**
     * @brief function to use in order to get the good result 
     * if we have updated the coefficient 1 or 0 of our 
     * polynomial
     * 
     * @param current_result_of_our_protocol result given by the server
     * @return libff::Fr<ppT> 
     */
    libff::Fr<ppT> protocol_result_updated(libff::Fr<ppT> current_result_of_our_protocol);

    /**
     * @brief Return our polynomial with the coefficients in the field ppT
     * 
     * @return vector<libff::Fr<ppT>> 
     */
    vector<libff::Fr<ppT>> get_polynomial();

    /**
     * @brief Get the coefficient of index 'coef' in our polynomial
     * 
     * @param coef 
     * @return libff::Fr<ppT> 
     */
    libff::Fr<ppT> get_polynomial_coefficients(uint64_t coef);

    /**
     * @brief Get the x variable 
     * 
     * @return pb_variable<FieldT> 
     */
    pb_variable<FieldT> get_x_variable();

    /**
     * @brief Get the y variable 
     * 
     * @return pb_variable<FieldT> 
     */
    pb_variable<FieldT> get_y_variable();

    /**
     * @brief Update the coeffcient of index 'index_of_coefficient' with 
     * his new value 'new_coef' in our polynomial
     * 
     * @param new_coef 
     * @param index_of_coefficient 
     */
    void update_polynomial_coefficient(libff::Fr<ppT> new_coef, uint64_t index_of_coefficient);

    /**
     * @brief Create a polynomials of degree "this->polynomial_degree" with random coefficient in the field "ppT".
     * 
     * @tparam ppT 
     * @param 
     * @return void
     */
    void create_random_coefficients_for_polynomial();

    /**
     * @brief Create the R1CS constraint for the polynomial 'this->poly' with the horner method (https://en.wikipedia.org/wiki/Horner%27s_method)
     * Defined as follow 
     * def horner(P,x): with P the coefficient of our polynomial and x our variable
    n=len(P)
    valeur=P[n-1]
    for i in range(n-2,-1,-1):
        valeur=valeur*x+P[i]
    return valeur
    * 
    * @tparam FieldT Field for the constraint
    * @tparam ppT Field of the coeffictient of the polynomial 'this->poly'
    */
    void create_constraint_horner_method();

    /**
     * @brief Just a function to check our computation with the R1CS. Compute the polynomial 'this->poly' on the 'this->x' 
     * variable value with the horner method
     * 
     * @tparam ppT The field of the polynomial
     * @param x_value The value of x
     * @return libff::Fr<ppT> The result of our computation
     */
    libff::Fr<ppT> evaluation_polynomial_horner();

    /**
     * @brief Update the coefficient p of index 'index_of_the_coefficient' to p'
     * with p'= p + 'delta'
     * 
     * @param index_of_the_coefficient 
     * @param delta 
     */
    void update_constraint_horner_method(uint64_t index_of_the_coefficient, libff::Fr<ppT> delta);

    /**
     * @brief Set the protoboard in our factory
     * 
     * @param protoboard_for_poly 
     */
    void set_protoboard(protoboard<FieldT>* protoboard_for_poly);

    /**
     * @brief Get the protoboard of the factory
     * 
     * @return protoboard<FieldT> 
     */
    protoboard<FieldT> get_protoboard();

    /**
     * @brief Set the constraint system of our factory
     * 
     * @param constraint_system 
     */
    void set_constraint_system(r1cs_constraint_system<FieldT> constraint_system);

    /**
     * @brief Function to update a QAP polynomial at one point to match the new coefficient
     * value.
     * 
     * @param FFT_evaluation_point 
     * @param QAPPoly 
     * @param new_polynomial_coefficient 
     * @param last_polynomial_coefficient 
     * @param index 
     * @return FieldT 
     */
    FieldT r1cs_to_qap_instance_map_with_evaluation_point(const FieldT &FFT_evaluation_point, FieldT QAPPoly, 
            FieldT new_polynomial_coefficient, FieldT last_polynomial_coefficient, uint64_t index);
    
    /**
     * @brief Function to create a wrong R1CS for the tests
     * 
     */
    void create_wrong_constraint_horner_method();
    
    /**
     * @brief Update the proving key to match our new coefficient in the R1CS
     * 
     * @param coef_index 
     * @param FFT_evaluation_point 
     * @return r1cs_ppzksnark_keypair<ppT> 
     */
    r1cs_ppzksnark_keypair<ppT> update_proving_key_compilation(uint64_t coef_index, libff::Fr<ppT> FFT_evaluation_point);

    /**
     * @brief Get the constraint system of our factory
     * 
     * @return r1cs_constraint_system<FieldT> 
     */
    r1cs_constraint_system<FieldT> get_constraint_system();

    /**
     * @brief Set the random container in our factory
     * 
     * @param random_container 
     */
    void set_random_container(random_container_key<ppT> random_container);

    /**
     * @brief Get the random container of the factory
     * 
     * @return random_container_key<ppT> 
     */
    random_container_key<ppT> get_random_container();

    /**
     * @brief Clear all the polynomial (this->poly) of our factory 
     * 
     */
    void clear_polynomial();

    /**
     * @brief Function to generate the pk and vk for the protocol
     * And allow to do the update by storing the evaluation point for our QAP
     * 
     * @return return_container_key_generator_for_update<ppT> 
     */
    return_container_key_generator_for_update<ppT> r1cs_ppzksnark_key_generator_for_update();

    /**
     * @brief Set the current key pair in the factory
     * 
     * @param key_pair 
     */
    void set_current_key_pair(r1cs_ppzksnark_keypair<ppT> *key_pair);
    
    /**
     * @brief Get the current key pair in the factory
     * 
     * @return r1cs_ppzksnark_keypair<ppT>* 
     */
    r1cs_ppzksnark_keypair<ppT>* get_current_key_pair();

    /**
     * @brief Change the R1CS stored in the current key pair in the factory
     * 
     * @param constraint_system_update 
     */
    void update_keypair_constraint_system(const r1cs_constraint_system<FieldT> constraint_system_update);

    /**
     * @brief Get the proving key in the factory
     * 
     * @return r1cs_ppzksnark_proving_key<ppT> 
     */
    r1cs_ppzksnark_proving_key<ppT> get_proving_key();

    /**
     * @brief Get the verification key in the factory
     * 
     * @return r1cs_ppzksnark_verification_key<ppT> 
     */
    r1cs_ppzksnark_verification_key<ppT> get_verification_key();

private:
    vector<libff::Fr<ppT>> poly;
    vector<libff::Fr<ppT>> poly_special_save;
    uint64_t polynomial_degree;
    r1cs_constraint_system<FieldT> constraint_system;

    pb_variable<FieldT> y;
    pb_variable<FieldT> x;
    protoboard<FieldT>* protoboard_for_poly;
    random_container_key<ppT> random_container;
    
    libff::Fr<ppT> coefficient_zero_updated;
    libff::Fr<ppT> coefficient_one_updated;

    libff::Fr<ppT> save_coefficient_zero;
    libff::Fr<ppT> save_coefficient_one;
    libff::Fr<ppT> x_value;

    libff::Fr<ppT> save_constraint_a_d_3_j_b_terms;
    libff::Fr<ppT> save_constraint_a_d_4_j_b_terms;
    libff::Fr<ppT> save_constraint_a_d_4_j_a_terms;
    libff::Fr<ppT> save_constraint_a_d_5_j_a_terms;

    libff::Fr<ppT> new_constraint_a_d_3_j_b_terms;
    libff::Fr<ppT> new_constraint_a_d_4_j_b_terms;
    libff::Fr<ppT> new_constraint_a_d_4_j_a_terms;
    libff::Fr<ppT> new_constraint_a_d_5_j_a_terms;

    r1cs_ppzksnark_keypair<ppT> *current_key_pair;

    /**
     * @brief This variable is just here for the test to insert error in the 
     * protocol depending of it's value;
     * 0 -> no error inserted
     * 1 -> wrong update of the R1CS
     * 2 -> wrong update of the key
     */
    int insert_error_for_test;

    bool already_changed_one_time;
};


#include "r1cs_from_poly_gen_function.tcc"

#endif 
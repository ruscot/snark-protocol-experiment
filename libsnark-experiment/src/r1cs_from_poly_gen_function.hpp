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

template<typename FieldT, typename ppT>
class R1CS_Polynomial_factory{
public:

    R1CS_Polynomial_factory(uint64_t degree, int insert_error_for_test);

    void update_coefficient_zero(libff::Fr<ppT> new_zero_coefficient_value, libff::Fr<ppT> last_zero_coefficient_value);

    void update_coefficient_one(libff::Fr<ppT> new_one_coefficient_value, libff::Fr<ppT> last_one_coefficient_value);

    void set_x_value(libff::Fr<ppT> x_value);

    uint64_t get_polynomial_degree();
    
    libff::Fr<ppT> protocol_result_updated(libff::Fr<ppT> current_result_of_our_protocol);

    /**
     * @brief Return our polynomial with the coefficients in the field ppT
     * 
     * @return vector<libff::Fr<ppT>> 
     */
    vector<libff::Fr<ppT>> get_polynomial();

    libff::Fr<ppT> get_polynomial_coefficients(uint64_t coef);

    pb_variable<FieldT> get_x_variable();

    pb_variable<FieldT> get_y_variable();

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
     * @brief Create the R1CS constraint for a given polynomial "poly" with the horner method (https://en.wikipedia.org/wiki/Horner%27s_method)
     * Defined as follow 
     * def horner(P,x): with P the coefficient of our polynomial and x our variable
    n=len(P)
    valeur=P[n-1]
    for i in range(n-2,-1,-1):
        valeur=valeur*x+P[i]
    return valeur
    * 
    * @tparam FieldT Field for the constraint
    * @tparam ppT Field of the coeffictient of the polynomial "poly"
    * @param poly Vector of coefficient of our polynomial
    * @param pb The protoboard who will stock our R1CS constraint
    * @param degree The degree of our polynomial
    * @return pb_variable<FieldT> 
    */
    void create_constraint_horner_method();

    /**
     * @brief Just a function to check our computation with the R1CS. Compute the polynomial on "x_value" with the horner method
     * 
     * @tparam ppT The field of the polynomial
     * @param x_value The value of x
     * @return libff::Fr<ppT> The result of our computation
     */
    libff::Fr<ppT> evaluation_polynomial_horner();

    void update_constraint_horner_method(uint64_t index_of_the_coefficient, libff::Fr<ppT> delta);

    void set_protoboard(protoboard<FieldT>* protoboard_for_poly);

    protoboard<FieldT> get_protoboard();

    void set_constraint_system(r1cs_constraint_system<FieldT> constraint_system);

    FieldT r1cs_to_qap_instance_map_with_evaluation_At(const FieldT &FFT_evaluation_point, FieldT At, 
                FieldT new_polynomial_coefficient, FieldT last_polynomial_coefficient, uint64_t index);

    FieldT r1cs_to_qap_instance_map_with_evaluation_Zt(FieldT Zt, FieldT At_save, FieldT new_At, 
                                                            FieldT rA, FieldT beta);
    
    void create_wrong_constraint_horner_method();
    
    r1cs_ppzksnark_keypair<ppT> update_proving_key_compilation(uint64_t coef_index, 
                        libff::Fr<ppT> FFT_evaluation_point);

    r1cs_constraint_system<FieldT> get_constraint_system();

    void set_random_container(random_container_key<ppT> random_container);

    random_container_key<ppT> get_random_container();

    void clear_polynomial();

    return_container_key_generator_for_update<ppT> r1cs_ppzksnark_key_generator_for_update();

    void set_current_key_pair(r1cs_ppzksnark_keypair<ppT> *key_pair);
    
    r1cs_ppzksnark_keypair<ppT>* get_current_key_pair();

    void update_keypair_constraint_system(const r1cs_constraint_system<FieldT> constraint_system_update);

    r1cs_ppzksnark_proving_key<ppT> get_proving_key();

    r1cs_ppzksnark_verification_key<ppT> get_verification_key();

private:
    vector<libff::Fr<ppT>> poly;
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
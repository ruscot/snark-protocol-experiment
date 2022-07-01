#include <stdlib.h>
#include <iostream>

#include "libff/algebra/fields/field_utils.hpp"
#include "libsnark/zk_proof_systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark.hpp"
#include "libsnark/common/default_types/r1cs_ppzksnark_pp.hpp"
#include "libsnark/gadgetlib1/pb_variable.hpp"

#include "libsnark/gadgetlib2/variable.hpp"
#include "libsnark/gadgetlib2/variable.cpp"

#include "util.hpp"
#include "r1cs_from_poly_gen_function.cpp"
#include "r1cs_evaluation.cpp"
#include "r1cs_update_keys.cpp"

#include <stdexcept>
#include <tuple>
#include <string> 

using namespace libsnark;
using namespace std;

//Some structures and definitions to get the time of the computation
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

template<typename FieldT, typename default_r1cs_ppzksnark_pp>
r1cs_variable_assignment<FieldT> compute_polynomial_witness_output(protoboard<FieldT> &protoboard_for_poly, 
    r1cs_ppzksnark_proving_key<default_r1cs_ppzksnark_pp> pk){
//Compute the witness and output of our polynomial
    r1cs_variable_assignment<FieldT> full_variable_assignment =  protoboard_for_poly.primary_input();
    r1cs_variable_assignment<FieldT> full_variable_assignment_update =  protoboard_for_poly.primary_input();
    full_variable_assignment.push_back(protoboard_for_poly.auxiliary_input()[0]);

    for(r1cs_constraint<FieldT> cs : pk.constraint_system.constraints){
        
        FieldT cValue = evaluation_on_linear_combination(cs.a, cs.b, full_variable_assignment);
        for (auto &lt : cs.c.terms)
        {
            if(lt.index != 0 )
            {
                pb_variable<FieldT> annex;
                annex.index = lt.index;
                protoboard_for_poly.val(annex) = cValue;
            }
        }
        full_variable_assignment.push_back(cValue);
    }

    return full_variable_assignment_update;
}


template<typename FieldT>
void test_polynomial_in_clear_update(r1cs_variable_assignment<FieldT> full_variable_assignment_update,
                                const r1cs_constraint_system<FieldT> constraint_system, 
                                std::tuple<r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp>,std::tuple<libff::Fr<default_r1cs_ppzksnark_pp>, 
                                random_container_key<default_r1cs_ppzksnark_pp>>> ret_val, const r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> keypair, uint64_t coef_to_update,
                                R1CS_Polynomial_factory<FieldT, default_r1cs_ppzksnark_pp> r1cs_polynomial_factory)
{
    libff::enter_block("test_polynomial_in_clear_update");
    double time_polynomial_coef_update=0., time_polynomial_horner_update=0., time_key_update=0.;
    if(coef_to_update > r1cs_polynomial_factory.get_polynomial_degree() || coef_to_update < 0){
        throw std::runtime_error("Coefficient of the polynomial not in the polynomial ");
    }
    Chrono c_setup; 
    /*Change one polynomial coefficient*/
    c_setup.start();
    libff::Fr<default_r1cs_ppzksnark_pp> save_last_value_of_the_coef = r1cs_polynomial_factory.get_polynomial_coefficients(coef_to_update);
    r1cs_polynomial_factory.update_polynomial_coefficient(libff::Fr<default_r1cs_ppzksnark_pp>::random_element(), coef_to_update);
    time_polynomial_coef_update = c_setup.stop();
    libff::Fr<default_r1cs_ppzksnark_pp> res = r1cs_polynomial_factory.evaluation_polynomial_horner();
    
    c_setup.start();
    r1cs_polynomial_factory.update_constraint_horner_method(coef_to_update);
    time_polynomial_horner_update = c_setup.stop();
    
    //Compute the witness and output of our polynomial
    protoboard<FieldT> protoboard_for_poly = r1cs_polynomial_factory.get_protoboard();
    full_variable_assignment_update.push_back(protoboard_for_poly.auxiliary_input()[0]);
    const r1cs_constraint_system<FieldT> constraint_system_update = protoboard_for_poly.get_constraint_system();
    for(r1cs_constraint<FieldT> cs : constraint_system_update.constraints){
        
        FieldT cValue = evaluation_on_linear_combination(cs.a, cs.b, full_variable_assignment_update);
        for (auto &lt : cs.c.terms)
        {
            if(lt.index != 0 )
            {
                pb_variable<FieldT> annex;
                annex.index = lt.index;
                protoboard_for_poly.val(annex) = cValue;
            }
        }
        full_variable_assignment_update.push_back(cValue);
    }
    c_setup.start();    
    uint64_t index = 0;
    if(coef_to_update == r1cs_polynomial_factory.get_polynomial_degree()){
        index = 0;
    } else if (coef_to_update == r1cs_polynomial_factory.get_polynomial_degree() - 1) {
        index = 1;
    } else {
        index = r1cs_polynomial_factory.get_polynomial_degree() - coef_to_update;
    }

    r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> test_res_keypair = update_proving_key_compilation(constraint_system, 
                                    save_last_value_of_the_coef, r1cs_polynomial_factory.get_random_polynomial()[coef_to_update], 
                                    index, r1cs_polynomial_factory.get_polynomial_degree(), std::get<0>(std::get<1>(ret_val)), std::get<1>(std::get<1>(ret_val)), keypair);
    time_key_update = c_setup.stop();
    test_res_keypair.pk.constraint_system = constraint_system_update;
    const r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof = r1cs_ppzksnark_prover<default_r1cs_ppzksnark_pp>(test_res_keypair.pk, 
                                            protoboard_for_poly.primary_input(), 
                                            protoboard_for_poly.auxiliary_input());
    bool verified = r1cs_ppzksnark_verifier_strong_IC<default_r1cs_ppzksnark_pp>(test_res_keypair.vk, protoboard_for_poly.primary_input(), proof);
    
    bool test = res == protoboard_for_poly.primary_input()[0];

    if(test == 0) {
        throw std::runtime_error("The result for polynomial eval is not correct abort");
    }

    if(verified == 0) {
        throw std::runtime_error("The proof is not correct abort");
    }
    cout << "test after update equal or not " << test << endl;

    r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> oldKeyPair = r1cs_ppzksnark_generator55<default_r1cs_ppzksnark_pp>(constraint_system, 
                                    save_last_value_of_the_coef, r1cs_polynomial_factory.get_random_polynomial()[coef_to_update], 
                                    coef_to_update, r1cs_polynomial_factory.get_polynomial_degree(), std::get<0>(std::get<1>(ret_val)), std::get<1>(std::get<1>(ret_val)) );

    

    r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> newkeypair = r1cs_ppzksnark_generator_with_t_and_random<default_r1cs_ppzksnark_pp>(
                                                    constraint_system_update, std::get<0>(std::get<1>(ret_val)),  
                                                    std::get<1>(std::get<1>(ret_val)), r1cs_polynomial_factory.get_random_polynomial()[coef_to_update], 
                                                    coef_to_update, r1cs_polynomial_factory.get_polynomial_degree() );

    cout << "Check if our keypair updated is correct " << endl;
    compare_keypair(newkeypair,test_res_keypair);
    printf("[TIMINGS for update] | %lu | polynomial coef update : %f | key update : %f | time_polynomial_horner_update : %f | total : %f \n=== end ===\n\n", 
        r1cs_polynomial_factory.get_polynomial_degree(), time_polynomial_coef_update, time_key_update, time_polynomial_horner_update, time_polynomial_horner_update + time_key_update + time_polynomial_coef_update);
    libff::leave_block("test_polynomial_in_clear_update");
}

void test_polynomial_in_clear(uint64_t degree){
    //Some variables to calculate the time of our computations
    double time_i=0., time_client=0., time_server=0. ;

    typedef libff::Fr<default_r1cs_ppzksnark_pp> FieldT;
    libff::enter_block("test_polynomial_in_clear");

    // Initialize the curve parameters
    default_r1cs_ppzksnark_pp::init_public_params();

    //Create our protoboard which will stock our R1CS    
    protoboard<FieldT> protoboard_for_poly;

    R1CS_Polynomial_factory<FieldT, default_r1cs_ppzksnark_pp> r1cs_polynomial_factory(degree);

    //Creation of the polynomial
    r1cs_polynomial_factory.create_random_coefficients_for_polynomial();
    vector<libff::Fr<default_r1cs_ppzksnark_pp>> polynomial = r1cs_polynomial_factory.get_random_polynomial();
    
    //Timer setup 
    Chrono c_setup; 
    /**
     * SETUP START
     */
    //Start the timer for the setup phase
    c_setup.start();
    //Create our R1CS constraint and get our input variable x
    r1cs_polynomial_factory.create_constraint_horner_method(&protoboard_for_poly);
    std::tuple<pb_variable<FieldT>,pb_variable<FieldT>> x_and_out = r1cs_polynomial_factory.get_out_and_in_variable();
    
    //Choose a random x on which we want to eval our polynomial
    protoboard_for_poly.val(std::get<0>(x_and_out)) = libff::Fr<default_r1cs_ppzksnark_pp>::random_element();
    protoboard_for_poly.val(std::get<1>(x_and_out)) = 0;
    protoboard_for_poly.set_input_sizes(1);
    const r1cs_constraint_system<FieldT> constraint_system = protoboard_for_poly.get_constraint_system();

    //Creation of our keys for the zkSNARK protocol from our R1CS constraints
    std::tuple<r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp>,std::tuple<libff::Fr<default_r1cs_ppzksnark_pp>, 
        random_container_key<default_r1cs_ppzksnark_pp>>> ret_val = r1cs_ppzksnark_generator2<default_r1cs_ppzksnark_pp>(constraint_system);

    const r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> keypair = std::get<0>(ret_val);
    //timer of the setup stop
    time_i = c_setup.stop();
    /**
     * SERVER START
     */
    c_setup.start();
    r1cs_variable_assignment<FieldT> full_variable_assignment_update = compute_polynomial_witness_output<FieldT, 
                    default_r1cs_ppzksnark_pp>(protoboard_for_poly, keypair.pk);

    //From our witnes and input output compute the proof for the client
    const r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof = r1cs_ppzksnark_prover<default_r1cs_ppzksnark_pp>(keypair.pk, 
                                            protoboard_for_poly.primary_input(), 
                                            protoboard_for_poly.auxiliary_input());
    //server part end
    time_server = c_setup.stop();
    /**
     * CLIENT START
     */
    c_setup.start();
    //Check that the proof send by the server is correct
    bool verified = r1cs_ppzksnark_verifier_strong_IC<default_r1cs_ppzksnark_pp>(keypair.vk, protoboard_for_poly.primary_input(), 
            proof);
    time_client = c_setup.stop();

    if(verified == 0) {
        throw std::runtime_error("The proof is not correct abort");
    }
    printf("[TIMINGS ] | %lu | setup : %f | audit-client : %f | audit-server : %f \n=== end ===\n\n", 
        degree+1, time_i, time_client, time_server);
    r1cs_polynomial_factory.set_protoboard(protoboard_for_poly);
    libff::Fr<default_r1cs_ppzksnark_pp> res = r1cs_polynomial_factory.evaluation_polynomial_horner();
    bool test = res == protoboard_for_poly.primary_input()[0];
    if(test == 0) {
        throw std::runtime_error("Result for the polynomial didn't match");
    }
    //Data store by the client 
    cout << "Data store by the client " << keypair.vk.size_in_bits()  << endl;
    //Data store by the server
    cout << "Data store by the server " << endl;
    cout << "   - Keys " << keypair.pk.size_in_bits() << endl;
    cout << "   - Total " <<  keypair.pk.size_in_bits() << endl;

    //Data sends by the client to the server at Init
    cout << "Data sends by the client to the server at Init " << endl;
    cout << "   - Keys " << keypair.pk.size_in_bits() << endl;
    cout << "   - Total " << keypair.pk.size_in_bits() << endl;
    //Data sends at eval
    cout << "Data sends at eval " << endl;
    cout << "   - Input value " << libff::size_in_bits(protoboard_for_poly.primary_input()) << endl;
    //Data sends back by server, return value of eval
    cout << "Data sends back by server, return value of eval " << endl;
    cout << "   - proof " << proof.size_in_bits() << endl;
    cout << "   - result " << libff::size_in_bits(protoboard_for_poly.primary_input()) << endl;
    cout << "   - Total " << libff::size_in_bits(protoboard_for_poly.primary_input()) +  proof.size_in_bits() << endl;
    test_polynomial_in_clear_update<FieldT>(full_variable_assignment_update, constraint_system, 
                                ret_val, keypair, 0 , r1cs_polynomial_factory);
    
    libff::leave_block("test_polynomial_in_clear");
}

int main(int argc, char * argv[])
{
    if (argc < 2) {
	    printf("Usage: main <Polynomial degree> \n");
	    return 1;
    }

    //The degree of our polynomial
    const uint64_t degree = atoi(argv[1]) - 1;
    test_polynomial_in_clear(degree);
    
    return 0;
}
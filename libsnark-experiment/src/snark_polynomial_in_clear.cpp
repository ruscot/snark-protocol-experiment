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

void test_polynomial_in_clear(uint64_t degree){
    //Some variables to calculate the time of our computations
    double time_i=0., time_client=0., time_server=0. ;

    typedef libff::Fr<default_r1cs_ppzksnark_pp> FieldT;
    libff::enter_block("test_polynomial_in_clear");

    // Initialize the curve parameters
    default_r1cs_ppzksnark_pp::init_public_params();

    //Create our protoboard which will stock our R1CS    
    protoboard<FieldT> protoboard_for_poly;
    //Creation of the polynomial
    vector<libff::Fr<default_r1cs_ppzksnark_pp>> polynomial = create_polynomials<default_r1cs_ppzksnark_pp>(degree);

    //Timer setup 
    Chrono c_setup; 
    /**
     * SETUP START
     */
    //Start the timer for the setup phase
    c_setup.start();
    //Create our R1CS constraint and get our input variable x
    std::tuple<pb_variable<FieldT>,pb_variable<FieldT>> x_and_out = create_constraint_horner_method<FieldT, default_r1cs_ppzksnark_pp>(polynomial, &protoboard_for_poly, degree);
    //Choose a random x on which we want to eval our polynomial
    protoboard_for_poly.val(std::get<0>(x_and_out)) = libff::Fr<default_r1cs_ppzksnark_pp>::random_element();
    protoboard_for_poly.val(std::get<1>(x_and_out)) = 0;
    protoboard_for_poly.set_input_sizes(1);
    const r1cs_constraint_system<FieldT> constraint_system = protoboard_for_poly.get_constraint_system();

    //Creation of our keys for the zkSNARK protocol from our R1CS constraints
    const r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> keypair = r1cs_ppzksnark_generator<default_r1cs_ppzksnark_pp>(constraint_system);
    
    //timer of the setup stop
    time_i = c_setup.stop();
    /**
     * SERVER START
     */
    c_setup.start();
    //Compute the witness and output of our polynomial
    r1cs_variable_assignment<FieldT> full_variable_assignment =  protoboard_for_poly.primary_input();
    full_variable_assignment.push_back(protoboard_for_poly.auxiliary_input()[0]);
    cout << "full variable assignement " << full_variable_assignment <<endl;
    libff::enter_block("Evaluation on linear combination");
    for(r1cs_constraint<FieldT> cs : constraint_system.constraints){
        
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

    //From our witnes and input output compute the proof for the client
    //protoboard_for_poly.set_input_sizes(1);
    const r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof = r1cs_ppzksnark_prover<default_r1cs_ppzksnark_pp>(keypair.pk, protoboard_for_poly.primary_input(), protoboard_for_poly.auxiliary_input());
    //server part end
    time_server = c_setup.stop();
    /**
     * CLIENT START
     */
    c_setup.start();
    //Check that the proof send by the server is correct
    bool verified = r1cs_ppzksnark_verifier_strong_IC<default_r1cs_ppzksnark_pp>(keypair.vk, protoboard_for_poly.primary_input(), proof);
    time_client = c_setup.stop();

    //cout << "Number of R1CS constraints: " << constraint_system.num_constraints() << endl;
    //cout << "Verification status: " << verified << endl;
    if(verified == 0) {
        throw std::runtime_error("The proof is not correct abort");
    }
    //cout << "Primary (public) input: " << protoboard_for_poly.primary_input() << endl;
    //cout << "Auxiliary (private) input poly : " << protoboard_for_poly.auxiliary_input()[protoboard_for_poly.auxiliary_input().size()-1] << endl;
    //cout << "Auxiliary (private) input poly : " << protoboard_for_poly.auxiliary_input() << endl;

    printf("[TIMINGS ] | %lu | setup : %f | audit-client : %f | audit-server : %f \n=== end ===\n\n", 
        degree+1, time_i, time_client, time_server);
    libff::Fr<default_r1cs_ppzksnark_pp> res = evaluation_polynomial_horner<default_r1cs_ppzksnark_pp>(polynomial, degree, protoboard_for_poly.auxiliary_input()[0]);
    //cout << "res = " << res <<endl;
    bool test = res == protoboard_for_poly.primary_input()[0];
    //cout << "verif poly eval is correct = " << test << endl;
    if(test == 0) {
        throw std::runtime_error("Result for the polynomial didn't match");
    }
    //Data store by the client 
    cout << "Data store by the client " << keypair.vk.size_in_bits()  << endl;
    //Data store by the server
    cout << "Data store by the server " << endl;
    cout << "   - Keys " << keypair.pk.size_in_bits() << endl;
    size_t size_constraint = sizeof(constraint_system.primary_input_size) * 8 + sizeof(constraint_system.auxiliary_input_size) * 8;
    for(r1cs_constraint<FieldT> cs : constraint_system.constraints){
        for(linear_term<FieldT> linearTerm: cs.a.terms){
            size_constraint += FieldT::size_in_bits();
            size_constraint += sizeof(linearTerm.index) * 8;
        }
        for(linear_term<FieldT> linearTerm: cs.b.terms){
            size_constraint += FieldT::size_in_bits();
            size_constraint += sizeof(linearTerm.index) * 8;
        }
        for(linear_term<FieldT> linearTerm: cs.c.terms){
            size_constraint += FieldT::size_in_bits();
            size_constraint += sizeof(linearTerm.index) * 8;
        }
    }
    cout << "   - Constraints size " << size_constraint << endl;
    cout << "   - Total " << size_constraint +  keypair.pk.size_in_bits() << endl;

    //Data sends by the client to the server at Init
    cout << "Data sends by the client to the server at Init " << endl;
    cout << "   - Keys " << keypair.pk.size_in_bits() << endl;
    size_constraint = sizeof(constraint_system.primary_input_size) * 8 + sizeof(constraint_system.auxiliary_input_size) * 8;
    for(r1cs_constraint<FieldT> cs : constraint_system.constraints){
        for(linear_term<FieldT> linearTerm: cs.a.terms){
            size_constraint += FieldT::size_in_bits();
            size_constraint += sizeof(linearTerm.index) * 8;
        }
        for(linear_term<FieldT> linearTerm: cs.b.terms){
            size_constraint += FieldT::size_in_bits();
            size_constraint += sizeof(linearTerm.index) * 8;
        }
        for(linear_term<FieldT> linearTerm: cs.c.terms){
            size_constraint += FieldT::size_in_bits();
            size_constraint += sizeof(linearTerm.index) * 8;
        }
    }
    cout << "   - Constraints size " << size_constraint << endl;
    cout << "   - Total " << size_constraint +  keypair.pk.size_in_bits() << endl;
    //Data sends at eval
    cout << "Data sends at eval " << endl;
    cout << "   - Input value " << libff::size_in_bits(protoboard_for_poly.primary_input()) << endl;
    //Data sends back by server, return value of eval
    cout << "Data sends back by server, return value of eval " << endl;
    cout << "   - proof " << proof.size_in_bits() << endl;
    cout << "   - result " << libff::size_in_bits(protoboard_for_poly.primary_input()) << endl;
    cout << "   - Total " << libff::size_in_bits(protoboard_for_poly.primary_input()) +  proof.size_in_bits() << endl;
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
/*
 * run_ppzksnark.cpp
 *
 *      Author: Ahmed Kosba
 */
#include <stdlib.h>
#include <iostream>
#include <typeinfo>

#include "libff/algebra/fields/field_utils.hpp"
#include "libsnark/zk_proof_systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark.hpp"
#include "libsnark/common/default_types/r1cs_ppzksnark_pp.hpp"
#include "libsnark/gadgetlib1/pb_variable.hpp"

#include "libsnark/gadgetlib2/variable.hpp"
#include "libsnark/gadgetlib2/variable.cpp"

#include <stdexcept>
#include <tuple>
#include <string> 

#include "CircuitReader.hpp"
#include <libsnark/gadgetlib2/integration.hpp>
#include <libsnark/gadgetlib2/adapters.hpp>
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_ppzksnark/examples/run_r1cs_ppzksnark.hpp>
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark.hpp>
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_gg_ppzksnark/examples/run_r1cs_gg_ppzksnark.hpp>
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_gg_ppzksnark/r1cs_gg_ppzksnark.hpp>
#include <libsnark/common/default_types/r1cs_gg_ppzksnark_pp.hpp>
#include "libff/algebra/fields/field_utils.hpp"
#include <libsnark/gadgetlib2/variable.hpp>
using namespace libsnark;
using namespace std;
typedef ::std::shared_ptr<Constraint> ConstraintPtr;

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

int main(int argc, char **argv) {
    //Some variables to calculate the time of our computations
    double time_i=0., time_client=0., time_server=0. ;

    typedef libff::Fr<default_r1cs_ppzksnark_pp> FieldT;
    libff::enter_block("test_polynomial_in_paillier");

    // Initialize the curve parameters
    default_r1cs_ppzksnark_pp::init_public_params();

    //Timer setup 
    Chrono c_setup;

    /**
     * SETUP START
     */
    //Start the timer for the setup phase
    //c_setup.start();

	libff::start_profiling();
	gadgetlib2::initPublicParamsFromDefaultPp();
	gadgetlib2::GadgetLibAdapter::resetVariableIndex();
	//Just creating the protoboard
	ProtoboardPtr pb_client = gadgetlib2::Protoboard::create(gadgetlib2::R1P);
	
	
	//Check for arguments
	int inputStartIndex = 0;
	if(argc == 4){
		if(strcmp(argv[1], "gg") != 0){
			cout << "Invalid Argument - Terminating.." << endl;
			return -1;
		} else{
			cout << "Using ppzsknark in the generic group model [Gro16]." << endl;
		}
		inputStartIndex = 1;	
	}
	c_setup.start();
	// Read the circuit, evaluate, and translate constraints
	CircuitReader reader_client(argv[1 + inputStartIndex], argv[2 + inputStartIndex], pb_client);
	r1cs_constraint_system<FieldT> cs_client = get_constraint_system_from_gadgetlib2(*pb_client);
	cs_client.primary_input_size = reader_client.getNumInputs() + reader_client.getNumOutputs();
    //Creation of our keys for the zkSNARK protocol from our R1CS constraints
    const r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> keypair = r1cs_ppzksnark_generator<default_r1cs_ppzksnark_pp>(cs_client);
    
    //timer of the setup stop
    time_i = c_setup.stop();
	
    /**
     * SERVER START
     */
    c_setup.start();
	ProtoboardPtr pb_server = gadgetlib2::Protoboard::create(gadgetlib2::R1P);
    //Compute the witness and output of our polynomial
	CircuitReader reader_server(argv[3 + inputStartIndex], argv[4 + inputStartIndex], pb_server);

	r1cs_constraint_system<FieldT> cs_server = get_constraint_system_from_gadgetlib2(*pb_server);
	const r1cs_variable_assignment<FieldT> full_assignment_server =
			get_variable_assignment_from_gadgetlib2(*pb_server);
	cs_server.primary_input_size = reader_server.getNumInputs() + reader_server.getNumOutputs();
	cs_server.auxiliary_input_size = full_assignment_server.size() - cs_server.num_inputs();

	ConstraintSystem cur_constraint_system = (*pb_server).constraintSystem();
    // extract primary and auxiliary input
	const r1cs_primary_input<FieldT> primary_input_server(full_assignment_server.begin(),
			full_assignment_server.begin() + cs_server.num_inputs());
	
	const r1cs_auxiliary_input<FieldT> auxiliary_input_server(
			full_assignment_server.begin() + cs_server.num_inputs(), full_assignment_server.end());

	const r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof = r1cs_ppzksnark_prover<default_r1cs_ppzksnark_pp>(keypair.pk, 
                                            primary_input_server, auxiliary_input_server);

	time_server = c_setup.stop();

	/**
     * CLIENT check proof
     */
	c_setup.start();
	bool verified = r1cs_ppzksnark_verifier_strong_IC<default_r1cs_ppzksnark_pp>(keypair.vk, primary_input_server, proof);
	time_client = c_setup.stop();

	if(verified == 0) {
        throw std::runtime_error("The proof is not correct abort");
    }
	printf("[TIMINGS ] | setup : %f | audit-client : %f | audit-server : %f \n=== end ===\n\n", 
        time_i, time_client, time_server);

    libff::leave_block("test_polynomial_in_paillier");
	return 0;
}



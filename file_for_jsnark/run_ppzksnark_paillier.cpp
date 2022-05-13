/*
 * run_ppzksnark.cpp
 *
 *      Author: Ahmed Kosba
 */
#include <stdlib.h>
#include <iostream>

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

template<typename FieldT>
FieldT evaluation_on_linear_combination(linear_combination<FieldT> a, linear_combination<FieldT> b, std::vector<FieldT> &assignment) {
    FieldT elemA = a.evaluate(assignment);
    FieldT elemB = b.evaluate(assignment);
    FieldT elemC = elemA * elemB;
	exit(0);
    return elemC;
}

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
    c_setup.start();

	libff::start_profiling();
	gadgetlib2::initPublicParamsFromDefaultPp();
	gadgetlib2::GadgetLibAdapter::resetVariableIndex();
	//Just creating the protoboard
	ProtoboardPtr pb = gadgetlib2::Protoboard::create(gadgetlib2::R1P);
	
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
	
	// Read the circuit, evaluate, and translate constraints
	CircuitReader reader(argv[1 + inputStartIndex], argv[2 + inputStartIndex], pb);
	r1cs_constraint_system<FieldT> cs = get_constraint_system_from_gadgetlib2(*pb);
	//try_to_compute_witness_from_gadgetlib2(*pb);
	cout << cs.constraints.size() << endl;
	//try_to_compute_witness_from_gadgetlib2(*pb);
	const r1cs_variable_assignment<FieldT> full_assignment =
			get_variable_assignment_from_gadgetlib2(*pb);
	cs.primary_input_size = reader.getNumInputs() + reader.getNumOutputs();
	cs.auxiliary_input_size = full_assignment.size() - cs.num_inputs();

    //Creation of our keys for the zkSNARK protocol from our R1CS constraints
    const r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> keypair = r1cs_ppzksnark_generator<default_r1cs_ppzksnark_pp>(cs);
    
    //timer of the setup stop
    time_i = c_setup.stop();
	
    /**
     * SERVER START
     */
    c_setup.start();
    //Compute the witness and output of our polynomial
    r1cs_variable_assignment<FieldT> full_variable_assignment(full_assignment.begin(),
			full_assignment.begin() + cs.num_inputs());
	ConstraintSystem cur_constraint_system = (*pb).constraintSystem();
	/*cout << "full assignement size : " << full_assignment.size() << endl;
    for(r1cs_constraint<FieldT> cs : keypair.pk.constraint_system.constraints){
        
        FieldT cValue = evaluation_on_linear_combination(cs.a, cs.b, full_variable_assignment);
        for (auto &lt : cs.c.terms)
        {
            if(lt.index != 0 )
            {
                Variable annex;
                //annex.index = lt.index;
                //(*pb).val(lt) = cValue;
            }
        }
        full_variable_assignment.push_back(cValue);
    }*/
    // extract primary and auxiliary input
	const r1cs_primary_input<FieldT> primary_input(full_assignment.begin(),
			full_assignment.begin() + cs.num_inputs());
	const r1cs_auxiliary_input<FieldT> auxiliary_input(
			full_assignment.begin() + cs.num_inputs(), full_assignment.end());
	// only print the circuit output values if both flags MONTGOMERY and BINARY outputs are off (see CMakeLists file)
	// In the default case, these flags should be ON for faster performance.

#if !defined(MONTGOMERY_OUTPUT) && !defined(OUTPUT_BINARY)
	cout << endl << "Printing output assignment in readable format:: " << endl;
	std::vector<Wire> outputList = reader.getOutputWireIds();
	int start = reader.getNumInputs();
	int end = reader.getNumInputs() +reader.getNumOutputs();	
	for (int i = start ; i < end; i++) {
		cout << "[output]" << " Value of Wire # " << outputList[i-reader.getNumInputs()] << " :: ";
		cout << primary_input[i];
		cout << endl;
	}
	cout << endl;
#endif

	//assert(cs.is_valid());

	// removed cs.is_valid() check due to a suspected (off by 1) issue in a newly added check in their method.
        // A follow-up will be added.
	if(!cs.is_satisfied(primary_input, auxiliary_input)){
		cout << "The constraint system is  not satisifed by the value assignment - Terminating." << endl;
		return -1;
	}

	r1cs_example<FieldT> example(cs, primary_input, auxiliary_input);
	
	const bool test_serialization = false;
	bool successBit = false;
	if(argc == 3) {
		successBit = libsnark::run_r1cs_ppzksnark<libff::default_ec_pp>(example, test_serialization);

	} else {
		// The following code makes use of the observation that 
		// libsnark::default_r1cs_gg_ppzksnark_pp is the same as libff::default_ec_pp (see r1cs_gg_ppzksnark_pp.hpp)
		// otherwise, the following code won't work properly, as GadgetLib2 is hardcoded to use libff::default_ec_pp.
		successBit = libsnark::run_r1cs_gg_ppzksnark<libsnark::default_r1cs_gg_ppzksnark_pp>(
			example, test_serialization);
	}

	if(!successBit){
		cout << "Problem occurred while running the ppzksnark algorithms .. " << endl;
		return -1;
	}	

    libff::leave_block("test_polynomial_in_paillier");
	return 0;
}



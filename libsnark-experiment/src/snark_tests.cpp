#include "snark_tests.hpp"

void test_update_index_0(uint64_t polynomial_degree) {
    libff::enter_block("test_update_index_0");
    typedef libff::Fr<default_r1cs_ppzksnark_pp> FieldT;
    //Container of our R1CS with our function on it 
    R1CS_Polynomial_factory<FieldT, default_r1cs_ppzksnark_pp> r1cs_polynomial_factory(polynomial_degree, 0);
    
    // Initialize the curve parameters
    default_r1cs_ppzksnark_pp::init_public_params();

    //Create our protoboard which will stock our R1CS corresponding to our polynomial 
    protoboard<FieldT> protoboard_for_poly;

    //Creation of a polynomial of degree polynomial_degree with random coefficients
    r1cs_polynomial_factory.create_random_coefficients_for_polynomial();
    vector<libff::Fr<default_r1cs_ppzksnark_pp>> polynomial = r1cs_polynomial_factory.get_polynomial();
    //Gives the protoboard to our factory
    r1cs_polynomial_factory.set_protoboard(&protoboard_for_poly);

    //Create our R1CS constraint and get our input variable x and our output variable y
    r1cs_polynomial_factory.create_constraint_horner_method();
    pb_variable<FieldT> x = r1cs_polynomial_factory.get_x_variable();
    pb_variable<FieldT> y = r1cs_polynomial_factory.get_y_variable();
    
    //Choose a random x on which we want to eval our polynomial
    libff::Fr<default_r1cs_ppzksnark_pp> x_value =  libff::Fr<default_r1cs_ppzksnark_pp>::random_element();
    protoboard_for_poly.val(x) = x_value;
    r1cs_polynomial_factory.set_x_value(x_value);
    protoboard_for_poly.val(y) = 0;
    protoboard_for_poly.set_input_sizes(1);
    
    //Creation of our keys for the zkSNARK protocol from our R1CS constraints
    return_container_key_generator_for_update<default_r1cs_ppzksnark_pp> element_for_update = 
            r1cs_polynomial_factory.r1cs_ppzksnark_key_generator_for_update();

    r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> keypair = element_for_update.get_key_pair();

    const r1cs_constraint_system<FieldT> constraint_system = protoboard_for_poly.get_constraint_system();

    //Commpute the witness and output of our R1CS (all the a_i)
    compute_polynomial_witness_output<FieldT, default_r1cs_ppzksnark_pp>(protoboard_for_poly, keypair.pk);

    //From our witnes and input output compute the proof for the client
    const r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof = r1cs_ppzksnark_prover<default_r1cs_ppzksnark_pp>(
            keypair.pk, protoboard_for_poly.primary_input(), protoboard_for_poly.auxiliary_input());

    //Check that the proof send by the server is correct
    bool verified = r1cs_ppzksnark_verifier_strong_IC<default_r1cs_ppzksnark_pp>(keypair.vk, protoboard_for_poly.primary_input(), proof);

    if(verified == 0) {
        throw std::runtime_error("The proof is not correct abort");
    }
    r1cs_polynomial_factory.set_protoboard(&protoboard_for_poly);

    //Check that the result is correct (verify that our protocol gives the good result)
    libff::Fr<default_r1cs_ppzksnark_pp> res = r1cs_polynomial_factory.evaluation_polynomial_horner();
    bool test = res == protoboard_for_poly.primary_input()[0];
    if(test == 0) {
        throw std::runtime_error("Result for the polynomial didn't match");
    }
    //Add our R1CS, the current keypair, and the element to update the R1CS to our factory
    r1cs_polynomial_factory.set_constraint_system(constraint_system);
    r1cs_polynomial_factory.set_random_container(element_for_update.get_random_container());
    r1cs_polynomial_factory.set_current_key_pair(&keypair);
    try {
        //Try to update the index 0 of our polynomial
        test_polynomial_in_clear_update<FieldT>( 
                element_for_update, 0, &r1cs_polynomial_factory);
    } catch(std::runtime_error& e) {
        string error_msg = e.what();
        throw std::runtime_error("Error catch during first update \"" + error_msg + "\"");
    }
    
    try {
        //Try to do another update on the coefficient 0 to see if it works too
        test_polynomial_in_clear_update<FieldT>( 
                element_for_update, 0, &r1cs_polynomial_factory);
    } catch(std::runtime_error& e) {
        string error_msg = e.what();
        throw std::runtime_error("Error catch during second update \"" + error_msg + "\"");
    }
    
    r1cs_polynomial_factory.clear_polynomial();
    libff::leave_block("test_update_index_0");
}

void test_update_index_1(uint64_t polynomial_degree) {
    libff::enter_block("test_update_index_1");
    typedef libff::Fr<default_r1cs_ppzksnark_pp> FieldT;
    //Container of our R1CS with our function on it 
    R1CS_Polynomial_factory<FieldT, default_r1cs_ppzksnark_pp> r1cs_polynomial_factory(polynomial_degree, 0);

    // Initialize the curve parameters
    default_r1cs_ppzksnark_pp::init_public_params();

    //Create our protoboard which will stock our R1CS corresponding to our polynomial 
    protoboard<FieldT> protoboard_for_poly;

    //Creation of a polynomial of degree polynomial_degree with random coefficients
    r1cs_polynomial_factory.create_random_coefficients_for_polynomial();
    vector<libff::Fr<default_r1cs_ppzksnark_pp>> polynomial = r1cs_polynomial_factory.get_polynomial();
    //Gives the protoboard to our factory
    r1cs_polynomial_factory.set_protoboard(&protoboard_for_poly);

    //Create our R1CS constraint and get our input variable x and our output variable y
    r1cs_polynomial_factory.create_constraint_horner_method();
    pb_variable<FieldT> x = r1cs_polynomial_factory.get_x_variable();
    pb_variable<FieldT> y = r1cs_polynomial_factory.get_y_variable();
    
    //Choose a random x on which we want to eval our polynomial
    libff::Fr<default_r1cs_ppzksnark_pp> x_value =  libff::Fr<default_r1cs_ppzksnark_pp>::random_element();
    protoboard_for_poly.val(x) = x_value;
    r1cs_polynomial_factory.set_x_value(x_value);
    protoboard_for_poly.val(y) = 0;
    protoboard_for_poly.set_input_sizes(1);
    
    //Creation of our keys for the zkSNARK protocol from our R1CS constraints
    return_container_key_generator_for_update<default_r1cs_ppzksnark_pp> element_for_update = 
            r1cs_polynomial_factory.r1cs_ppzksnark_key_generator_for_update();

    r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> keypair = element_for_update.get_key_pair();

    const r1cs_constraint_system<FieldT> constraint_system = protoboard_for_poly.get_constraint_system();
    
    //Commpute the witness and output of our R1CS (all the a_i)
    compute_polynomial_witness_output<FieldT, default_r1cs_ppzksnark_pp>(protoboard_for_poly, keypair.pk);

    //From our witnes and input output compute the proof for the client
    const r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof = r1cs_ppzksnark_prover<default_r1cs_ppzksnark_pp>(
            keypair.pk, protoboard_for_poly.primary_input(), protoboard_for_poly.auxiliary_input());

    //Check that the proof send by the server is correct
    bool verified = r1cs_ppzksnark_verifier_strong_IC<default_r1cs_ppzksnark_pp>(keypair.vk, protoboard_for_poly.primary_input(), proof);

    if(verified == 0) {
        throw std::runtime_error("The proof is not correct abort");
    }
    r1cs_polynomial_factory.set_protoboard(&protoboard_for_poly);

    //Check that the result is correct (verify that our protocol gives the good result)
    libff::Fr<default_r1cs_ppzksnark_pp> res = r1cs_polynomial_factory.evaluation_polynomial_horner();
    bool test = res == protoboard_for_poly.primary_input()[0];
    if(test == 0) {
        throw std::runtime_error("Result for the polynomial didn't match");
    }
    //Add our R1CS, the current keypair, and the element to update the R1CS to our factory
    r1cs_polynomial_factory.set_constraint_system(constraint_system);
    r1cs_polynomial_factory.set_random_container(element_for_update.get_random_container());
    r1cs_polynomial_factory.set_current_key_pair(&keypair);
    try {
        //Try to update the index 1 of our polynomial
        test_polynomial_in_clear_update<FieldT>( 
                element_for_update, 1, &r1cs_polynomial_factory);
    } catch(std::runtime_error& e) {
        string error_msg = e.what();
        throw std::runtime_error("Error catch during first update \"" + error_msg + "\"");
    }
    
    try {
        //Try to do another update on the coefficient 1 to see if it works too
        test_polynomial_in_clear_update<FieldT>( 
                element_for_update, 1, &r1cs_polynomial_factory);
    } catch(std::runtime_error& e) {
        string error_msg = e.what();
        throw std::runtime_error("Error catch during second update \"" + error_msg + "\"");
    }
    
    r1cs_polynomial_factory.clear_polynomial();
    libff::leave_block("test_update_index_1");
}

void test_update_random_index(uint64_t polynomial_degree, int random_index_to_update) {
    libff::enter_block("test_update_random_index");
    typedef libff::Fr<default_r1cs_ppzksnark_pp> FieldT;
    //Container of our R1CS with our function on it 
    R1CS_Polynomial_factory<FieldT, default_r1cs_ppzksnark_pp> r1cs_polynomial_factory(polynomial_degree, 0);

    // Initialize the curve parameters
    default_r1cs_ppzksnark_pp::init_public_params();

    //Create our protoboard which will stock our R1CS corresponding to our polynomial 
    protoboard<FieldT> protoboard_for_poly;

    //Creation of a polynomial of degree polynomial_degree with random coefficients
    r1cs_polynomial_factory.create_random_coefficients_for_polynomial();
    vector<libff::Fr<default_r1cs_ppzksnark_pp>> polynomial = r1cs_polynomial_factory.get_polynomial();
    //Gives the protoboard to our factory
    r1cs_polynomial_factory.set_protoboard(&protoboard_for_poly);

    //Create our R1CS constraint and get our input variable x and our output variable y
    r1cs_polynomial_factory.create_constraint_horner_method();
    pb_variable<FieldT> x = r1cs_polynomial_factory.get_x_variable();
    pb_variable<FieldT> y = r1cs_polynomial_factory.get_y_variable();
    
    //Choose a random x on which we want to eval our polynomial
    libff::Fr<default_r1cs_ppzksnark_pp> x_value =  libff::Fr<default_r1cs_ppzksnark_pp>::random_element();
    protoboard_for_poly.val(x) = x_value;
    r1cs_polynomial_factory.set_x_value(x_value);
    protoboard_for_poly.val(y) = 0;
    protoboard_for_poly.set_input_sizes(1);
    
    //Creation of our keys for the zkSNARK protocol from our R1CS constraints
    return_container_key_generator_for_update<default_r1cs_ppzksnark_pp> element_for_update = 
            r1cs_polynomial_factory.r1cs_ppzksnark_key_generator_for_update();

    r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> keypair = element_for_update.get_key_pair();

    const r1cs_constraint_system<FieldT> constraint_system = protoboard_for_poly.get_constraint_system();

    //Commpute the witness and output of our R1CS (all the a_i)
    compute_polynomial_witness_output<FieldT, default_r1cs_ppzksnark_pp>(protoboard_for_poly, keypair.pk);

    //From our witnes and input output compute the proof for the client
    const r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof = r1cs_ppzksnark_prover<default_r1cs_ppzksnark_pp>(
            keypair.pk, protoboard_for_poly.primary_input(), protoboard_for_poly.auxiliary_input());

    //Check that the proof send by the server is correct
    bool verified = r1cs_ppzksnark_verifier_strong_IC<default_r1cs_ppzksnark_pp>(keypair.vk, protoboard_for_poly.primary_input(), proof);

    if(verified == 0) {
        throw std::runtime_error("The proof is not correct abort");
    }
    r1cs_polynomial_factory.set_protoboard(&protoboard_for_poly);

    //Check that the result is correct (verify that our protocol gives the good result)
    libff::Fr<default_r1cs_ppzksnark_pp> res = r1cs_polynomial_factory.evaluation_polynomial_horner();
    bool test = res == protoboard_for_poly.primary_input()[0];
    if(test == 0) {
        throw std::runtime_error("Result for the polynomial didn't match");
    }
    //Add our R1CS, the current keypair, and the element to update the R1CS to our factory
    r1cs_polynomial_factory.set_constraint_system(constraint_system);
    r1cs_polynomial_factory.set_random_container(element_for_update.get_random_container());
    r1cs_polynomial_factory.set_current_key_pair(&keypair);
    try {
        //Try to update the index random_index_to_update of our polynomial
        test_polynomial_in_clear_update<FieldT>( 
                element_for_update, random_index_to_update, &r1cs_polynomial_factory);
    } catch(std::runtime_error& e) {
        string error_msg = e.what();
        throw std::runtime_error("Error catch during first update \"" + error_msg + "\"");
    }
    
    try {
        //Try to do another update on the coefficient random_index_to_update to see if it works too
        test_polynomial_in_clear_update<FieldT>( 
                element_for_update, random_index_to_update, &r1cs_polynomial_factory);
    } catch(std::runtime_error& e) {
        string error_msg = e.what();
        throw std::runtime_error("Error catch during second update \"" + error_msg + "\"");
    }
    
    r1cs_polynomial_factory.clear_polynomial();
    libff::leave_block("test_update_random_index");
}

void test_raise_error_for_wrong_r1cs_construction(uint64_t polynomial_degree) {
    libff::enter_block("test_raise_error_for_wrong_r1cs_construction");
    typedef libff::Fr<default_r1cs_ppzksnark_pp> FieldT;
    //Container of our R1CS with our function on it, we give the parameter values 1 to add an error on the R1CS during the test
    R1CS_Polynomial_factory<FieldT, default_r1cs_ppzksnark_pp> r1cs_polynomial_factory(polynomial_degree, 1);

    // Initialize the curve parameters
    default_r1cs_ppzksnark_pp::init_public_params();

    //Create our protoboard which will stock our R1CS corresponding to our polynomial 
    protoboard<FieldT> protoboard_for_poly;

    //Creation of a polynomial of degree polynomial_degree with random coefficients
    r1cs_polynomial_factory.create_random_coefficients_for_polynomial();
    vector<libff::Fr<default_r1cs_ppzksnark_pp>> polynomial = r1cs_polynomial_factory.get_polynomial();
    //Gives the protoboard to our factory
    r1cs_polynomial_factory.set_protoboard(&protoboard_for_poly);

    //Create our R1CS constraint and get our input variable x and our output variable y
    r1cs_polynomial_factory.create_wrong_constraint_horner_method();
    pb_variable<FieldT> x = r1cs_polynomial_factory.get_x_variable();
    pb_variable<FieldT> y = r1cs_polynomial_factory.get_y_variable();
    
    //Choose a random x on which we want to eval our polynomial
    libff::Fr<default_r1cs_ppzksnark_pp> x_value =  libff::Fr<default_r1cs_ppzksnark_pp>::random_element();
    protoboard_for_poly.val(x) = x_value;
    r1cs_polynomial_factory.set_x_value(x_value);
    protoboard_for_poly.val(y) = 0;
    protoboard_for_poly.set_input_sizes(1);
    
    //Creation of our keys for the zkSNARK protocol from our R1CS constraints
    return_container_key_generator_for_update<default_r1cs_ppzksnark_pp> element_for_update = 
            r1cs_polynomial_factory.r1cs_ppzksnark_key_generator_for_update();

    r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> keypair = element_for_update.get_key_pair();

    const r1cs_constraint_system<FieldT> constraint_system = protoboard_for_poly.get_constraint_system();

    //Commpute the witness and output of our R1CS (all the a_i)
    compute_polynomial_witness_output<FieldT, default_r1cs_ppzksnark_pp>(protoboard_for_poly, keypair.pk);

    //From our witnes and input output compute the proof for the client
    const r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof = r1cs_ppzksnark_prover<default_r1cs_ppzksnark_pp>(
            keypair.pk, protoboard_for_poly.primary_input(), protoboard_for_poly.auxiliary_input());

    //Check that the proof send by the server is correct
    bool verified = r1cs_ppzksnark_verifier_strong_IC<default_r1cs_ppzksnark_pp>(keypair.vk, protoboard_for_poly.primary_input(), proof);

    if(verified == 0) {
        throw std::runtime_error("The proof is not correct abort");
    }
    r1cs_polynomial_factory.set_protoboard(&protoboard_for_poly);

    //Check that the result is correct (verify that our protocol gives the good result)
    libff::Fr<default_r1cs_ppzksnark_pp> res = r1cs_polynomial_factory.evaluation_polynomial_horner();
    bool test = res == protoboard_for_poly.primary_input()[0];
    if(test == 0) {
        throw std::runtime_error("Result for the polynomial didn't match");
    }
    //Add our R1CS, the current keypair, and the element to update the R1CS to our factory
    r1cs_polynomial_factory.set_constraint_system(constraint_system);
    r1cs_polynomial_factory.set_random_container(element_for_update.get_random_container());
    r1cs_polynomial_factory.set_current_key_pair(&keypair);
    try {
        //Try to update the index 0 of our polynomial
        vector<double> timings_for_update = test_polynomial_in_clear_update<FieldT>( 
                element_for_update, 0, &r1cs_polynomial_factory);
    } catch(std::runtime_error& e) {
        string error_msg = e.what();
        throw std::runtime_error("Error catch during first update \"" + error_msg + "\"");
    }
    
    try {
        //Try to do another update on the coefficient 0 to see if it works too
        test_polynomial_in_clear_update<FieldT>( 
                element_for_update, 0, &r1cs_polynomial_factory);
    } catch(std::runtime_error& e) {
        string error_msg = e.what();
        throw std::runtime_error("Error catch during second update \"" + error_msg + "\"");
    }
    
    r1cs_polynomial_factory.clear_polynomial();
    libff::leave_block("test_raise_error_for_wrong_r1cs_construction");
}

void test_raise_error_for_wrong_r1cs_update(uint64_t polynomial_degree, int random_index_to_update) {
    libff::enter_block("test_raise_error_for_wrong_r1cs_update");
    typedef libff::Fr<default_r1cs_ppzksnark_pp> FieldT;
    //Container of our R1CS with our function on it, we give the parameter values 1 to add an error on the R1CS during the test
    R1CS_Polynomial_factory<FieldT, default_r1cs_ppzksnark_pp> r1cs_polynomial_factory(polynomial_degree, 1);

    // Initialize the curve parameters
    default_r1cs_ppzksnark_pp::init_public_params();

    //Create our protoboard which will stock our R1CS corresponding to our polynomial 
    protoboard<FieldT> protoboard_for_poly;

    //Creation of a polynomial of degree polynomial_degree with random coefficients
    r1cs_polynomial_factory.create_random_coefficients_for_polynomial();
    vector<libff::Fr<default_r1cs_ppzksnark_pp>> polynomial = r1cs_polynomial_factory.get_polynomial();
    //Gives the protoboard to our factory
    r1cs_polynomial_factory.set_protoboard(&protoboard_for_poly);

    //Create our R1CS constraint and get our input variable x and our output variable y
    r1cs_polynomial_factory.create_constraint_horner_method();
    pb_variable<FieldT> x = r1cs_polynomial_factory.get_x_variable();
    pb_variable<FieldT> y = r1cs_polynomial_factory.get_y_variable();
    
    //Choose a random x on which we want to eval our polynomial
    libff::Fr<default_r1cs_ppzksnark_pp> x_value =  libff::Fr<default_r1cs_ppzksnark_pp>::random_element();
    protoboard_for_poly.val(x) = x_value;
    r1cs_polynomial_factory.set_x_value(x_value);
    protoboard_for_poly.val(y) = 0;
    protoboard_for_poly.set_input_sizes(1);
    
    //Creation of our keys for the zkSNARK protocol from our R1CS constraints
    return_container_key_generator_for_update<default_r1cs_ppzksnark_pp> element_for_update = 
            r1cs_polynomial_factory.r1cs_ppzksnark_key_generator_for_update();

    r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> keypair = element_for_update.get_key_pair();

    const r1cs_constraint_system<FieldT> constraint_system = protoboard_for_poly.get_constraint_system();

    //Commpute the witness and output of our R1CS (all the a_i)
    compute_polynomial_witness_output<FieldT, default_r1cs_ppzksnark_pp>(protoboard_for_poly, keypair.pk);

    //From our witnes and input output compute the proof for the client
    const r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof = r1cs_ppzksnark_prover<default_r1cs_ppzksnark_pp>(
            keypair.pk, protoboard_for_poly.primary_input(), protoboard_for_poly.auxiliary_input());

    //Check that the proof send by the server is correct
    bool verified = r1cs_ppzksnark_verifier_strong_IC<default_r1cs_ppzksnark_pp>(keypair.vk, protoboard_for_poly.primary_input(), proof);

    if(verified == 0) {
        throw std::runtime_error("The proof is not correct abort");
    }
    r1cs_polynomial_factory.set_protoboard(&protoboard_for_poly);

    //Check that the result is correct (verify that our protocol gives the good result)
    libff::Fr<default_r1cs_ppzksnark_pp> res = r1cs_polynomial_factory.evaluation_polynomial_horner();
    bool test = res == protoboard_for_poly.primary_input()[0];
    if(test == 0) {
        throw std::runtime_error("Result for the polynomial didn't match");
    }
    //Add our R1CS, the current keypair, and the element to update the R1CS to our factory
    r1cs_polynomial_factory.set_constraint_system(constraint_system);
    r1cs_polynomial_factory.set_random_container(element_for_update.get_random_container());
    r1cs_polynomial_factory.set_current_key_pair(&keypair);
    try { 
        //Try to update the index random_index_to_update of our polynomial
        test_polynomial_in_clear_update<FieldT>( 
                element_for_update, random_index_to_update, &r1cs_polynomial_factory);
    } catch(std::runtime_error& e) {
        string error_msg = e.what();
        throw std::runtime_error("Error catch during first update \"" + error_msg + "\"");
    }
    
    try {
        //Try to do another update on the coefficient random_index_to_update to see if it works too
        test_polynomial_in_clear_update<FieldT>( 
                element_for_update, random_index_to_update, &r1cs_polynomial_factory);
    } catch(std::runtime_error& e) {
        string error_msg = e.what();
        throw std::runtime_error("Error catch during second update \"" + error_msg + "\"");
    }

    r1cs_polynomial_factory.clear_polynomial();
    libff::leave_block("test_raise_error_for_wrong_r1cs_update");
}

void test_raise_error_for_wrong_keys_update(uint64_t polynomial_degree, int random_index_to_update) {
    libff::enter_block("test_raise_error_for_wrong_keys_update");
    typedef libff::Fr<default_r1cs_ppzksnark_pp> FieldT;
    //Container of our R1CS with our function on it, we give the parameter values 2 to add an error on the keys update during the test
    R1CS_Polynomial_factory<FieldT, default_r1cs_ppzksnark_pp> r1cs_polynomial_factory(polynomial_degree, 2);

    // Initialize the curve parameters
    default_r1cs_ppzksnark_pp::init_public_params();

    //Create our protoboard which will stock our R1CS corresponding to our polynomial 
    protoboard<FieldT> protoboard_for_poly;

    //Creation of a polynomial of degree polynomial_degree with random coefficients
    r1cs_polynomial_factory.create_random_coefficients_for_polynomial();
    vector<libff::Fr<default_r1cs_ppzksnark_pp>> polynomial = r1cs_polynomial_factory.get_polynomial();
    //Gives the protoboard to our factory
    r1cs_polynomial_factory.set_protoboard(&protoboard_for_poly);

    //Create our R1CS constraint and get our input variable x and our output variable y
    r1cs_polynomial_factory.create_constraint_horner_method();
    pb_variable<FieldT> x = r1cs_polynomial_factory.get_x_variable();
    pb_variable<FieldT> y = r1cs_polynomial_factory.get_y_variable();
    
    //Choose a random x on which we want to eval our polynomial
    libff::Fr<default_r1cs_ppzksnark_pp> x_value =  libff::Fr<default_r1cs_ppzksnark_pp>::random_element();
    protoboard_for_poly.val(x) = x_value;
    r1cs_polynomial_factory.set_x_value(x_value);
    protoboard_for_poly.val(y) = 0;
    protoboard_for_poly.set_input_sizes(1);
    
    //Creation of our keys for the zkSNARK protocol from our R1CS constraints
    return_container_key_generator_for_update<default_r1cs_ppzksnark_pp> element_for_update = 
            r1cs_polynomial_factory.r1cs_ppzksnark_key_generator_for_update();

    r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> keypair = element_for_update.get_key_pair();

    const r1cs_constraint_system<FieldT> constraint_system = protoboard_for_poly.get_constraint_system();

    //Commpute the witness and output of our R1CS (all the a_i)
    compute_polynomial_witness_output<FieldT, default_r1cs_ppzksnark_pp>(protoboard_for_poly, keypair.pk);

    //From our witnes and input output compute the proof for the client
    const r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof = r1cs_ppzksnark_prover<default_r1cs_ppzksnark_pp>(
            keypair.pk, protoboard_for_poly.primary_input(), protoboard_for_poly.auxiliary_input());

    //Check that the proof send by the server is correct
    bool verified = r1cs_ppzksnark_verifier_strong_IC<default_r1cs_ppzksnark_pp>(keypair.vk, protoboard_for_poly.primary_input(), proof);

    if(verified == 0) {
        throw std::runtime_error("The proof is not correct abort");
    }
    r1cs_polynomial_factory.set_protoboard(&protoboard_for_poly);

    //Check that the result is correct (verify that our protocol gives the good result)
    libff::Fr<default_r1cs_ppzksnark_pp> res = r1cs_polynomial_factory.evaluation_polynomial_horner();
    bool test = res == protoboard_for_poly.primary_input()[0];
    if(test == 0) {
        throw std::runtime_error("Result for the polynomial didn't match");
    }
    //Add our R1CS, the current keypair, and the element to update the R1CS to our factory
    r1cs_polynomial_factory.set_constraint_system(constraint_system);
    r1cs_polynomial_factory.set_random_container(element_for_update.get_random_container());
    r1cs_polynomial_factory.set_current_key_pair(&keypair);
    try {
        //Try to update the index random_index_to_update of our polynomial
        test_polynomial_in_clear_update<FieldT>( 
                element_for_update, random_index_to_update, &r1cs_polynomial_factory);
    } catch(std::runtime_error& e) {
        string error_msg = e.what();
        throw std::runtime_error("Error catch during first update \"" + error_msg + "\"");
    }
    
    try {
        //Try to do another update on the coefficient random_index_to_update to see if it works too
        test_polynomial_in_clear_update<FieldT>( 
                element_for_update, random_index_to_update, &r1cs_polynomial_factory);
    } catch(std::runtime_error& e) {
        string error_msg = e.what();
        throw std::runtime_error("Error catch during second update \"" + error_msg + "\"");
    }
    
    r1cs_polynomial_factory.clear_polynomial();
    libff::leave_block("test_raise_error_for_wrong_keys_update");
}

void test_update_multiple_index(uint64_t polynomial_degree, vector<int> random_index_to_update) {
    libff::enter_block("test_update_multiple_index");
    typedef libff::Fr<default_r1cs_ppzksnark_pp> FieldT;
    //Container of our R1CS with our function on it 
    R1CS_Polynomial_factory<FieldT, default_r1cs_ppzksnark_pp> r1cs_polynomial_factory(polynomial_degree, 0);

    // Initialize the curve parameters
    default_r1cs_ppzksnark_pp::init_public_params();

    //Create our protoboard which will stock our R1CS corresponding to our polynomial 
    protoboard<FieldT> protoboard_for_poly;

    //Creation of a polynomial of degree polynomial_degree with random coefficients
    r1cs_polynomial_factory.create_random_coefficients_for_polynomial();
    vector<libff::Fr<default_r1cs_ppzksnark_pp>> polynomial = r1cs_polynomial_factory.get_polynomial();
    //Gives the protoboard to our factory
    r1cs_polynomial_factory.set_protoboard(&protoboard_for_poly);

    //Create our R1CS constraint and get our input variable x and our output variable y
    r1cs_polynomial_factory.create_constraint_horner_method();
    pb_variable<FieldT> x = r1cs_polynomial_factory.get_x_variable();
    pb_variable<FieldT> y = r1cs_polynomial_factory.get_y_variable();
    
    //Choose a random x on which we want to eval our polynomial
    libff::Fr<default_r1cs_ppzksnark_pp> x_value =  libff::Fr<default_r1cs_ppzksnark_pp>::random_element();
    protoboard_for_poly.val(x) = x_value;
    r1cs_polynomial_factory.set_x_value(x_value);
    protoboard_for_poly.val(y) = 0;
    protoboard_for_poly.set_input_sizes(1);
    
    //Creation of our keys for the zkSNARK protocol from our R1CS constraints
    return_container_key_generator_for_update<default_r1cs_ppzksnark_pp> element_for_update = 
            r1cs_polynomial_factory.r1cs_ppzksnark_key_generator_for_update();

    r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> keypair = element_for_update.get_key_pair();

    const r1cs_constraint_system<FieldT> constraint_system = protoboard_for_poly.get_constraint_system();

    //Commpute the witness and output of our R1CS (all the a_i)
    compute_polynomial_witness_output<FieldT, default_r1cs_ppzksnark_pp>(protoboard_for_poly, keypair.pk);

    //From our witnes and input output compute the proof for the client
    const r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof = r1cs_ppzksnark_prover<default_r1cs_ppzksnark_pp>(
            keypair.pk, protoboard_for_poly.primary_input(), protoboard_for_poly.auxiliary_input());

    //Check that the proof send by the server is correct
    bool verified = r1cs_ppzksnark_verifier_strong_IC<default_r1cs_ppzksnark_pp>(keypair.vk, protoboard_for_poly.primary_input(), proof);

    if(verified == 0) {
        throw std::runtime_error("The proof is not correct abort");
    }
    r1cs_polynomial_factory.set_protoboard(&protoboard_for_poly);

    //Check that the result is correct (verify that our protocol gaves the good result)
    libff::Fr<default_r1cs_ppzksnark_pp> res = r1cs_polynomial_factory.evaluation_polynomial_horner();
    bool test = res == protoboard_for_poly.primary_input()[0];
    if(test == 0) {
        throw std::runtime_error("Result for the polynomial didn't match");
    }
    //Add our R1CS, the current keypair, and the element to update the R1CS to our factory
    r1cs_polynomial_factory.set_constraint_system(constraint_system);
    r1cs_polynomial_factory.set_random_container(element_for_update.get_random_container());
    r1cs_polynomial_factory.set_current_key_pair(&keypair);
    for(size_t i = 0; i < random_index_to_update.size(); i++) {
        try {
            //Try to update the index random_index_to_update[i] of our polynomial
            test_polynomial_in_clear_update<FieldT>( 
                    element_for_update, random_index_to_update[i], &r1cs_polynomial_factory);
        } catch(std::runtime_error& e) {
            string error_msg = e.what();
            throw std::runtime_error("Error catch during the " + to_string(i) + "th update for coefficient index " + to_string(random_index_to_update[i]) + " \"" + error_msg + "\"");
        }
    }
    
    r1cs_polynomial_factory.clear_polynomial();
    libff::leave_block("test_update_multiple_index");
}

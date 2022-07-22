#include "snark_polynomial_in_clear.hpp"

template<typename FieldT, typename default_r1cs_ppzksnark_pp>
void compute_polynomial_witness_output(protoboard<FieldT> &protoboard_for_poly, 
    r1cs_ppzksnark_proving_key<default_r1cs_ppzksnark_pp> pk){
    r1cs_variable_assignment<FieldT> full_variable_assignment =  protoboard_for_poly.primary_input();
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
}



template<typename FieldT>
vector<double> test_polynomial_in_clear_update(
                                return_container_key_generator_for_update<default_r1cs_ppzksnark_pp> container_for_update, 
                                r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> last_keypair, uint64_t index_of_the_coef_to_update,
                                R1CS_Polynomial_factory<FieldT, default_r1cs_ppzksnark_pp> *r1cs_polynomial_factory)
{
    libff::enter_block("test_polynomial_in_clear_update");
    double time_polynomial_coef_update=0., time_polynomial_horner_update=0., time_key_update=0.;
    if(index_of_the_coef_to_update > (*r1cs_polynomial_factory).get_polynomial_degree() || index_of_the_coef_to_update < 0){
        throw std::runtime_error("Coefficient of the polynomial not in the polynomial ");
    }
    if(index_of_the_coef_to_update <= 1){
        //Special case where we don't update our R1CS and we just save the new coefficients
        Chrono c_setup; 
        c_setup.start();
        /*Change one polynomial coefficient*/
        libff::Fr<default_r1cs_ppzksnark_pp> new_coef = libff::Fr<default_r1cs_ppzksnark_pp>::random_element();
        libff::Fr<default_r1cs_ppzksnark_pp> save_last_value_of_the_coef = (*r1cs_polynomial_factory).get_polynomial_coefficients(index_of_the_coef_to_update);
        if(index_of_the_coef_to_update == 1){
            (*r1cs_polynomial_factory).update_one_coefficient(new_coef, save_last_value_of_the_coef);
        } else {
            (*r1cs_polynomial_factory).update_zero_coefficient(new_coef, save_last_value_of_the_coef);
        }

        (*r1cs_polynomial_factory).update_polynomial_coefficient(new_coef, index_of_the_coef_to_update);
        time_polynomial_coef_update = c_setup.stop();

        libff::Fr<default_r1cs_ppzksnark_pp> result_with_horners_method = (*r1cs_polynomial_factory).evaluation_polynomial_horner();
        
        //Compute the witness and output of our polynomial
        protoboard<FieldT> protoboard_for_poly = (*r1cs_polynomial_factory).get_protoboard();
        r1cs_variable_assignment<FieldT> full_variable_assignment_update =  protoboard_for_poly.primary_input();
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

        last_keypair.pk.constraint_system = constraint_system_update;
        const r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof = r1cs_ppzksnark_prover<default_r1cs_ppzksnark_pp>(last_keypair.pk, 
                                                protoboard_for_poly.primary_input(), 
                                                protoboard_for_poly.auxiliary_input());
        bool verified = r1cs_ppzksnark_verifier_strong_IC<default_r1cs_ppzksnark_pp>(last_keypair.vk, protoboard_for_poly.primary_input(), proof);
        
        libff::Fr<default_r1cs_ppzksnark_pp> result_of_protocol = (*r1cs_polynomial_factory).protocol_result_updated(protoboard_for_poly.primary_input()[0]);
        bool test = result_with_horners_method == result_of_protocol ;

        if(test == 0) {
            throw std::runtime_error("The result for polynomial eval is not correct abort");
        }

        if(verified == 0) {
            throw std::runtime_error("The proof is not correct abort");
        }
        
        printf("[TIMINGS for update] | %lu | polynomial coef update : %f | key update : %f | time_polynomial_horner_update : %f | total : %f \n=== end ===\n\n", 
            (*r1cs_polynomial_factory).get_polynomial_degree(), time_polynomial_coef_update, time_key_update, time_polynomial_horner_update, time_polynomial_horner_update + time_key_update + time_polynomial_coef_update);
        libff::leave_block("test_polynomial_in_clear_update");
        vector<double> timings {time_polynomial_coef_update, time_key_update, time_polynomial_horner_update, time_polynomial_horner_update + time_key_update + time_polynomial_coef_update};
        return timings;
    } else {
        Chrono c_setup; 
        c_setup.start();
        /*Change one polynomial coefficient*/
        libff::Fr<default_r1cs_ppzksnark_pp> new_coef_value = libff::Fr<default_r1cs_ppzksnark_pp>::random_element();
        cout << "new coef value ---------------------------------------------" << new_coef_value << endl;
        libff::Fr<default_r1cs_ppzksnark_pp> save_last_value_of_the_coef = (*r1cs_polynomial_factory).get_polynomial_coefficients(index_of_the_coef_to_update);
        cout << "save_last_value_of_the_coef ---------------------------------------------" << save_last_value_of_the_coef << endl;
        (*r1cs_polynomial_factory).update_polynomial_coefficient(new_coef_value, index_of_the_coef_to_update);
        time_polynomial_coef_update = c_setup.stop();

        libff::Fr<default_r1cs_ppzksnark_pp> result_with_horners_method = (*r1cs_polynomial_factory).evaluation_polynomial_horner();
        
        c_setup.start();
        libff::Fr<default_r1cs_ppzksnark_pp> delta = new_coef_value - save_last_value_of_the_coef;
        (*r1cs_polynomial_factory).update_constraint_horner_method_aude_version(index_of_the_coef_to_update, delta);
        time_polynomial_horner_update = c_setup.stop();
        
        //Compute the witness and output of our polynomial
        protoboard<FieldT> protoboard_for_poly = (*r1cs_polynomial_factory).get_protoboard();
        r1cs_variable_assignment<FieldT> full_variable_assignment_update =  protoboard_for_poly.primary_input();
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
        uint64_t index_in_the_r1cs = (*r1cs_polynomial_factory).get_polynomial_degree() - index_of_the_coef_to_update;

        r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> test_res_keypair = (*r1cs_polynomial_factory).update_proving_key_compilation( 
                        index_in_the_r1cs, container_for_update.get_FFT_evaluation_point(), last_keypair);

        time_key_update = c_setup.stop();

        test_res_keypair.pk.constraint_system = constraint_system_update;
        const r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof = r1cs_ppzksnark_prover<default_r1cs_ppzksnark_pp>(test_res_keypair.pk, 
                                                protoboard_for_poly.primary_input(), 
                                                protoboard_for_poly.auxiliary_input());
        bool verified = r1cs_ppzksnark_verifier_strong_IC<default_r1cs_ppzksnark_pp>(test_res_keypair.vk, protoboard_for_poly.primary_input(), proof);
        
        bool test = result_with_horners_method == protoboard_for_poly.primary_input()[0];

        if(test == 0) {
            throw std::runtime_error("The result for polynomial eval is not correct abort");
        }

        if(verified == 0) {
            throw std::runtime_error("The proof is not correct abort");
        }
        //Test after update to check if our key is correct
        std::tuple<r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp>, libff::Fr_vector<default_r1cs_ppzksnark_pp>> foo = r1cs_ppzksnark_generator_with_FFT_evaluation_point_and_random_values<default_r1cs_ppzksnark_pp>(
                                                        constraint_system_update, container_for_update.get_FFT_evaluation_point(),  
                                                        container_for_update.get_random_container());
        r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> newkeypair = std::get<0>(foo);
        libff::Fr_vector<default_r1cs_ppzksnark_pp> At_save = std::get<1>(foo);
        (*r1cs_polynomial_factory).compare_at(At_save, container_for_update.get_FFT_evaluation_point(), index_in_the_r1cs);
        //Check if our key pair is correct
        compare_keypair(newkeypair,test_res_keypair);
        
        printf("[TIMINGS for update] | %lu | polynomial coef update : %f | key update : %f | time_polynomial_horner_update : %f | total : %f \n=== end ===\n\n", 
            (*r1cs_polynomial_factory).get_polynomial_degree(), time_polynomial_coef_update, time_key_update, time_polynomial_horner_update, time_polynomial_horner_update + time_key_update + time_polynomial_coef_update);
        libff::leave_block("test_polynomial_in_clear_update");
        vector<double> timings {time_polynomial_coef_update, time_key_update, time_polynomial_horner_update, time_polynomial_horner_update + time_key_update + time_polynomial_coef_update};
        return timings;
    }
}

void test_polynomial_in_clear(uint64_t polynomial_degree, int number_of_try){
    double time_i_sum=0., 
        time_client_sum=0., 
        time_server_sum=0.,
         time_polynomial_coef_update_sum=0., 
        time_key_update_sum=0., 
        time_polynomial_horner_update_sum=0., 
        sum_update_timing_sum=0. ;
    //Create the timer to evaluate our function computation time
    Chrono c_setup; 
    //Some variables to calculate the time of our computations for one turn
    double time_i=0., 
        time_client=0., 
        time_server=0. ;
    typedef libff::Fr<default_r1cs_ppzksnark_pp> FieldT;
    //Container of our R1CS with our function on it 
    R1CS_Polynomial_factory<FieldT, default_r1cs_ppzksnark_pp> r1cs_polynomial_factory(polynomial_degree);
    for(int current_try = 0; current_try < number_of_try; current_try++){
        libff::enter_block("test_polynomial_in_clear");

        // Initialize the curve parameters
        default_r1cs_ppzksnark_pp::init_public_params();

        //Create our protoboard which will stock our R1CS corresponding to our polynomial 
        protoboard<FieldT> protoboard_for_poly;

        //Creation of a polynomial of degree polynomial_degree with random coefficients
        r1cs_polynomial_factory.create_random_coefficients_for_polynomial();
        vector<libff::Fr<default_r1cs_ppzksnark_pp>> polynomial = r1cs_polynomial_factory.get_polynomial();
        r1cs_polynomial_factory.set_protoboard(&protoboard_for_poly);

        /**
         * SETUP PHASES
         */
        //Start the timer for the setup phase
        c_setup.start();
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

        const r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> keypair = element_for_update.get_key_pair();
        //timer of the setup stop
        time_i = c_setup.stop();
        time_i_sum += time_i;

        /**
         * SERVER PHASES
         */
        const r1cs_constraint_system<FieldT> constraint_system = protoboard_for_poly.get_constraint_system();

        c_setup.start();
        compute_polynomial_witness_output<FieldT, default_r1cs_ppzksnark_pp>(protoboard_for_poly, keypair.pk);

        //From our witnes and input output compute the proof for the client
        const r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof = r1cs_ppzksnark_prover<default_r1cs_ppzksnark_pp>(
                                                keypair.pk, 
                                                protoboard_for_poly.primary_input(), 
                                                protoboard_for_poly.auxiliary_input());
        //server part end
        time_server = c_setup.stop();
        time_server_sum += time_server;
        /**
         * CLIENT START
         */
        c_setup.start();
        //Check that the proof send by the server is correct
        bool verified = r1cs_ppzksnark_verifier_strong_IC<default_r1cs_ppzksnark_pp>(keypair.vk, protoboard_for_poly.primary_input(), proof);
        time_client = c_setup.stop();
        time_client_sum += time_client;

        if(verified == 0) {
            throw std::runtime_error("The proof is not correct abort");
        }

        printf("[TIMINGS ] | current try : %d | %lu | setup : %f | audit-client : %f | audit-server : %f \n=== end ===\n\n", 
            current_try, polynomial_degree+1, time_i, time_client, time_server);
        r1cs_polynomial_factory.set_protoboard(&protoboard_for_poly);

        //Check that the result is correct (verify that our protocol gaves the good result)
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
        r1cs_polynomial_factory.set_constraint_system(constraint_system);
        r1cs_polynomial_factory.set_random_container(element_for_update.get_random_container());

        vector<double> timings_for_update = test_polynomial_in_clear_update<FieldT>( 
                                    element_for_update, keypair, 5, &r1cs_polynomial_factory);
        //Try to do another update to see if it works too
        test_polynomial_in_clear_update<FieldT>( 
                                    element_for_update, keypair, 5, &r1cs_polynomial_factory);

        //Get the timing of the update to perform a mean
        time_polynomial_coef_update_sum += timings_for_update[0];
        time_key_update_sum += timings_for_update[1];
        time_polynomial_horner_update_sum += timings_for_update[2];
        sum_update_timing_sum += timings_for_update[3];
        
        r1cs_polynomial_factory.clear_polynomial();
        libff::leave_block("test_polynomial_in_clear");
    }
    printf("[TIMINGS MOY] | %lu | setup : %f | audit-client : %f | audit-server : %f \n=== end ===\n\n", 
            polynomial_degree+1, time_i_sum/number_of_try, time_client_sum/number_of_try, time_server_sum/number_of_try);
    printf("[TIMINGS MOY for update] | %lu | polynomial coef update : %f | key update : %f | time_polynomial_horner_update : %f | total : %f \n=== end ===\n\n", 
        polynomial_degree+1, time_polynomial_coef_update_sum/number_of_try, time_key_update_sum/number_of_try, 
        time_polynomial_horner_update_sum/number_of_try, sum_update_timing_sum/number_of_try);
}

void test_update_index_0(uint64_t polynomial_degree){
    typedef libff::Fr<default_r1cs_ppzksnark_pp> FieldT;
    //Container of our R1CS with our function on it 
    R1CS_Polynomial_factory<FieldT, default_r1cs_ppzksnark_pp> r1cs_polynomial_factory(polynomial_degree);
    libff::enter_block("test_polynomial_in_clear");

    // Initialize the curve parameters
    default_r1cs_ppzksnark_pp::init_public_params();

    //Create our protoboard which will stock our R1CS corresponding to our polynomial 
    protoboard<FieldT> protoboard_for_poly;

    //Creation of a polynomial of degree polynomial_degree with random coefficients
    r1cs_polynomial_factory.create_random_coefficients_for_polynomial();
    vector<libff::Fr<default_r1cs_ppzksnark_pp>> polynomial = r1cs_polynomial_factory.get_polynomial();
    r1cs_polynomial_factory.set_protoboard(&protoboard_for_poly);

    /**
     * SETUP PHASES
     */
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

    const r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> keypair = element_for_update.get_key_pair();

    /**
     * SERVER PHASES
     */
    const r1cs_constraint_system<FieldT> constraint_system = protoboard_for_poly.get_constraint_system();

    compute_polynomial_witness_output<FieldT, default_r1cs_ppzksnark_pp>(protoboard_for_poly, keypair.pk);

    //From our witnes and input output compute the proof for the client
    const r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof = r1cs_ppzksnark_prover<default_r1cs_ppzksnark_pp>(
                                            keypair.pk, 
                                            protoboard_for_poly.primary_input(), 
                                            protoboard_for_poly.auxiliary_input());
    //server part end
    /**
     * CLIENT START
     */
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

    r1cs_polynomial_factory.set_constraint_system(constraint_system);
    r1cs_polynomial_factory.set_random_container(element_for_update.get_random_container());
    try{
        vector<double> timings_for_update = test_polynomial_in_clear_update<FieldT>( 
                                element_for_update, keypair, 0, &r1cs_polynomial_factory);
    }catch(std::runtime_error& e) {
        string error_msg = e.what();
        throw std::runtime_error("Error catch during first update \"" + error_msg + "\"");
    }
    
    try{
        //Try to do another update to see if it works too
        test_polynomial_in_clear_update<FieldT>( 
                                element_for_update, keypair, 0, &r1cs_polynomial_factory);
    }catch(std::runtime_error& e) {
        string error_msg = e.what();
        throw std::runtime_error("Error catch during second update \"" + error_msg + "\"");
    }
    
    r1cs_polynomial_factory.clear_polynomial();
    libff::leave_block("test_polynomial_in_clear");
}

void test_update_index_1(uint64_t polynomial_degree){
    typedef libff::Fr<default_r1cs_ppzksnark_pp> FieldT;
    //Container of our R1CS with our function on it 
    R1CS_Polynomial_factory<FieldT, default_r1cs_ppzksnark_pp> r1cs_polynomial_factory(polynomial_degree);
    libff::enter_block("test_polynomial_in_clear");

    // Initialize the curve parameters
    default_r1cs_ppzksnark_pp::init_public_params();

    //Create our protoboard which will stock our R1CS corresponding to our polynomial 
    protoboard<FieldT> protoboard_for_poly;

    //Creation of a polynomial of degree polynomial_degree with random coefficients
    r1cs_polynomial_factory.create_random_coefficients_for_polynomial();
    vector<libff::Fr<default_r1cs_ppzksnark_pp>> polynomial = r1cs_polynomial_factory.get_polynomial();
    r1cs_polynomial_factory.set_protoboard(&protoboard_for_poly);

    /**
     * SETUP PHASES
     */
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

    const r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> keypair = element_for_update.get_key_pair();

    /**
     * SERVER PHASES
     */
    const r1cs_constraint_system<FieldT> constraint_system = protoboard_for_poly.get_constraint_system();

    compute_polynomial_witness_output<FieldT, default_r1cs_ppzksnark_pp>(protoboard_for_poly, keypair.pk);

    //From our witnes and input output compute the proof for the client
    const r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof = r1cs_ppzksnark_prover<default_r1cs_ppzksnark_pp>(
                                            keypair.pk, 
                                            protoboard_for_poly.primary_input(), 
                                            protoboard_for_poly.auxiliary_input());
    //server part end
    /**
     * CLIENT START
     */
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

    r1cs_polynomial_factory.set_constraint_system(constraint_system);
    r1cs_polynomial_factory.set_random_container(element_for_update.get_random_container());
    try{
        vector<double> timings_for_update = test_polynomial_in_clear_update<FieldT>( 
                                element_for_update, keypair, 1, &r1cs_polynomial_factory);
    }catch(std::runtime_error& e) {
        string error_msg = e.what();
        throw std::runtime_error("Error catch during first update \"" + error_msg + "\"");
    }
    
    try{
        //Try to do another update to see if it works too
        test_polynomial_in_clear_update<FieldT>( 
                                element_for_update, keypair, 1, &r1cs_polynomial_factory);
    }catch(std::runtime_error& e) {
        string error_msg = e.what();
        throw std::runtime_error("Error catch during second update \"" + error_msg + "\"");
    }
    
    
    r1cs_polynomial_factory.clear_polynomial();
    libff::leave_block("test_polynomial_in_clear");
}

void test_update_random_index(uint64_t polynomial_degree, int random_index_to_update){
    typedef libff::Fr<default_r1cs_ppzksnark_pp> FieldT;
    //Container of our R1CS with our function on it 
    R1CS_Polynomial_factory<FieldT, default_r1cs_ppzksnark_pp> r1cs_polynomial_factory(polynomial_degree);
    libff::enter_block("test_polynomial_in_clear");

    // Initialize the curve parameters
    default_r1cs_ppzksnark_pp::init_public_params();

    //Create our protoboard which will stock our R1CS corresponding to our polynomial 
    protoboard<FieldT> protoboard_for_poly;

    //Creation of a polynomial of degree polynomial_degree with random coefficients
    r1cs_polynomial_factory.create_random_coefficients_for_polynomial();
    vector<libff::Fr<default_r1cs_ppzksnark_pp>> polynomial = r1cs_polynomial_factory.get_polynomial();
    r1cs_polynomial_factory.set_protoboard(&protoboard_for_poly);

    /**
     * SETUP PHASES
     */
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

    const r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> keypair = element_for_update.get_key_pair();

    /**
     * SERVER PHASES
     */
    const r1cs_constraint_system<FieldT> constraint_system = protoboard_for_poly.get_constraint_system();

    compute_polynomial_witness_output<FieldT, default_r1cs_ppzksnark_pp>(protoboard_for_poly, keypair.pk);

    //From our witnes and input output compute the proof for the client
    const r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof = r1cs_ppzksnark_prover<default_r1cs_ppzksnark_pp>(
                                            keypair.pk, 
                                            protoboard_for_poly.primary_input(), 
                                            protoboard_for_poly.auxiliary_input());
    //server part end
    /**
     * CLIENT START
     */
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

    r1cs_polynomial_factory.set_constraint_system(constraint_system);
    r1cs_polynomial_factory.set_random_container(element_for_update.get_random_container());
    
    try{
        vector<double> timings_for_update = test_polynomial_in_clear_update<FieldT>( 
                                element_for_update, keypair, random_index_to_update, &r1cs_polynomial_factory);
    }catch(std::runtime_error& e) {
        string error_msg = e.what();
        throw std::runtime_error("Error catch during first update \"" + error_msg + "\"");
    }
    
    try{
        //Try to do another update to see if it works too
        test_polynomial_in_clear_update<FieldT>( 
                                element_for_update, keypair, random_index_to_update, &r1cs_polynomial_factory);
    }catch(std::runtime_error& e) {
        string error_msg = e.what();
        throw std::runtime_error("Error catch during second update \"" + error_msg + "\"");
    }
    
    
    r1cs_polynomial_factory.clear_polynomial();
    libff::leave_block("test_polynomial_in_clear");
}
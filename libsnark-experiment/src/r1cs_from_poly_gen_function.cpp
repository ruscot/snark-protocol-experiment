#include "r1cs_from_poly_gen_function.hpp"
template<typename FieldT, typename ppT>
class R1CS_Polynomial_factory{
public:

    R1CS_Polynomial_factory(uint64_t degree){
        this->polynomial_degree = degree;
        this->protoboard_for_poly = NULL;
        this->coefficient_one_updated = libff::Fr<ppT>::zero();
        this->coefficient_zero_updated = libff::Fr<ppT>::zero();
    }

    void update_zero_coefficient(libff::Fr<ppT> new_zero_coefficient_value, libff::Fr<ppT> last_zero_coefficient_value){
        this->coefficient_zero_updated = new_zero_coefficient_value;
        this->save_coefficient_zero = last_zero_coefficient_value;
    }

    void update_one_coefficient(libff::Fr<ppT> new_one_coefficient_value, libff::Fr<ppT> last_one_coefficient_value){
        this->coefficient_one_updated = new_one_coefficient_value;
        this->save_coefficient_one = last_one_coefficient_value;
    }

    void set_x_value(libff::Fr<ppT> x_value){
        this->x_value = x_value;
    }

    uint64_t get_polynomial_degree(){
        return this->polynomial_degree;
    }
    
    libff::Fr<ppT> protocol_result_updated(libff::Fr<ppT> current_result_of_our_protocol){
        if(this->coefficient_one_updated != libff::Fr<ppT>::zero()){
            current_result_of_our_protocol += this->coefficient_one_updated * this->x_value - this->save_coefficient_one * this->x_value;
        }
        if(this->coefficient_zero_updated != libff::Fr<ppT>::zero()){
            current_result_of_our_protocol += this->coefficient_zero_updated - this->save_coefficient_zero;
        }

        return current_result_of_our_protocol;
    }

    /**
     * @brief Return our polynomial with the coefficients in the field ppT
     * 
     * @return vector<libff::Fr<ppT>> 
     */
    vector<libff::Fr<ppT>> get_polynomial(){
        return this->poly;
    }

    libff::Fr<ppT> get_polynomial_coefficients(uint64_t coef){
        return this->poly[coef];
    }

    pb_variable<FieldT> get_x_variable(){
        return this->x;
    }

    pb_variable<FieldT> get_y_variable(){
        return this->y;
    }

    void update_polynomial_coefficient(libff::Fr<ppT> new_coef, uint64_t index_of_coefficient){
        this->poly[index_of_coefficient] = new_coef;
    }

    /**
     * @brief Create a polynomials of degree "this->polynomial_degree" with random coefficient in the field "ppT".
     * 
     * @tparam ppT 
     * @param 
     * @return void
     */
    void create_random_coefficients_for_polynomial(){
        for(size_t i = 0; i <= this->polynomial_degree; ++i) {
            this->poly.push_back(libff::Fr<ppT>::random_element());
        }
    }

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
    void create_constraint_horner_method()
    {
        libff::enter_block("Create constraint horner method");
        this->y.allocate(*this->protoboard_for_poly, "out");
        this->x.allocate(*this->protoboard_for_poly, "x");

        if(this->polynomial_degree == 0){
            printf("Not a polynomial\n");
        } else {
            pb_variable<FieldT> last_var_1;
            last_var_1.allocate(*this->protoboard_for_poly, "0");
            (*this->protoboard_for_poly).add_r1cs_constraint(r1cs_constraint<FieldT>(this->poly[this->polynomial_degree] + 0 * this->x, this->x, last_var_1));
            string last_var_name = "1";
            for(uint64_t i = this->polynomial_degree; i > 1; i-=1){
                pb_variable<FieldT> last_var_2;
                last_var_name = std::to_string(stoi(last_var_name) + 1);
                last_var_2.allocate(*this->protoboard_for_poly, last_var_name);
                (*this->protoboard_for_poly).add_r1cs_constraint(r1cs_constraint<FieldT>(this->poly[i-1] + last_var_1 + 0 * this->x, this->x, last_var_2));
                last_var_1 = last_var_2;
            }
            (*this->protoboard_for_poly).add_r1cs_constraint(r1cs_constraint<FieldT>(this->poly[0] + last_var_1 + 0 * this->x, 1, this->y));
        }
        libff::leave_block("Create constraint horner method");
        
    }

    /**
     * @brief Just a function to check our computation with the R1CS. Compute the polynomial on "x_value" with the horner method
     * 
     * @tparam ppT The field of the polynomial
     * @param x_value The value of x
     * @return libff::Fr<ppT> The result of our computation
     */
    libff::Fr<ppT> evaluation_polynomial_horner() {
        libff::Fr<ppT> x_value = (*this->protoboard_for_poly).auxiliary_input()[0];
        libff::Fr<ppT> res = this->poly[this->polynomial_degree];
        for(uint64_t i = this->polynomial_degree; i > 0; i-=1){
            res = res*x_value + this->poly[i-1];
        }
        return res;
    }

    void update_constraint_horner_method(uint64_t index_of_the_coefficient)
    {
        libff::enter_block("Update constraint");      
        uint64_t index_in_the_r1cs = this->polynomial_degree - index_of_the_coefficient;
        r1cs_constraint<FieldT> constraint = (*this->protoboard_for_poly).get_specific_constraint_in_r1cs(index_in_the_r1cs);
        constraint.a.terms[0].coeff = this->poly[index_of_the_coefficient];
        (*this->protoboard_for_poly).protoboard_update_r1cs_constraint(constraint, index_in_the_r1cs);
        libff::leave_block("Update constraint");
    }

    void update_constraint_horner_method_aude_version(uint64_t index_of_the_coefficient, libff::Fr<ppT> delta)
    {
        libff::enter_block("Update constraint");      
        uint64_t index_in_the_r1cs = this->polynomial_degree - index_of_the_coefficient;
        FieldT random_k = libff::Fr<ppT>::random_element();
        r1cs_constraint<FieldT> constraint_a_d_3_j = (*this->protoboard_for_poly).get_specific_constraint_in_r1cs(index_in_the_r1cs);
        this->save_constraint_a_d_3_j_b_terms = constraint_a_d_3_j.b.terms[0].coeff;
        constraint_a_d_3_j.b.terms[0].coeff *= random_k;
        this->new_constraint_a_d_3_j_b_terms = constraint_a_d_3_j.b.terms[0].coeff;
        cout << "index of constraint_a_d_3_j" << endl;
        cout << constraint_a_d_3_j.b.terms[0].index << endl;
        r1cs_constraint<FieldT> constraint_a_d_4_j = (*this->protoboard_for_poly).get_specific_constraint_in_r1cs(
                                                                                index_in_the_r1cs + 1);
        this->save_constraint_a_d_4_j_a_terms = constraint_a_d_4_j.a.terms[1].coeff;
        constraint_a_d_4_j.a.terms[1].coeff = constraint_a_d_4_j.a.terms[1].coeff * random_k +
                                                delta * constraint_a_d_3_j.b.terms[0].coeff; 
        this->new_constraint_a_d_4_j_a_terms = constraint_a_d_4_j.a.terms[1].coeff;
        cout << "index of constraint_a_d_4_j" << endl;
        cout << constraint_a_d_4_j.a.terms[1].index << endl;
        FieldT constraint_a_d_4_j_b_save = constraint_a_d_4_j.b.terms[0].coeff;
        this->save_constraint_a_d_4_j_b_terms = constraint_a_d_4_j.b.terms[0].coeff;
        constraint_a_d_4_j.b.terms[0].coeff *= random_k.inverse();
        this->new_constraint_a_d_4_j_b_terms = constraint_a_d_4_j.b.terms[0].coeff;
        cout << "index of constraint_a_d_4_j" << endl;
        cout << constraint_a_d_4_j.b.terms[0].index << endl;
        r1cs_constraint<FieldT> constraint_a_d_5_j = (*this->protoboard_for_poly).get_specific_constraint_in_r1cs(index_in_the_r1cs + 2);
        this->save_constraint_a_d_5_j_a_terms = constraint_a_d_5_j.a.terms[1].coeff;
        constraint_a_d_5_j.a.terms[1].coeff += (libff::Fr<ppT>::one() - random_k.inverse()) * constraint_a_d_4_j.a.terms[0].coeff * 
                                                    constraint_a_d_4_j_b_save;
        this->new_constraint_a_d_5_j_a_terms = constraint_a_d_5_j.a.terms[1].coeff;
        cout << "index of constraint_a_d_5_j" << endl;
        cout << constraint_a_d_5_j.a.terms[1].index << endl;
        (*this->protoboard_for_poly).protoboard_update_r1cs_constraint(constraint_a_d_3_j, index_in_the_r1cs);
        (*this->protoboard_for_poly).protoboard_update_r1cs_constraint(constraint_a_d_4_j, index_in_the_r1cs + 1);
        (*this->protoboard_for_poly).protoboard_update_r1cs_constraint(constraint_a_d_5_j, index_in_the_r1cs + 2);
        
        libff::leave_block("Update constraint");
    }

    void set_protoboard(protoboard<FieldT>* protoboard_for_poly){
        this->protoboard_for_poly = protoboard_for_poly;
    }

    protoboard<FieldT> get_protoboard(){
        return (*this->protoboard_for_poly);
    }

    void set_constraint_system(r1cs_constraint_system<FieldT> constraint_system){
        this->constraint_system = constraint_system;
    }

    FieldT r1cs_to_qap_instance_map_with_evaluation_At(const FieldT &FFT_evaluation_point, FieldT At, 
                FieldT new_polynomial_coefficient, FieldT last_polynomial_coefficient, uint64_t index)
    {
        libff::enter_block("Call to r1cs_to_qap_instance_map_with_evaluation_At");
        const std::shared_ptr<libfqfft::evaluation_domain<FieldT> > domain = libfqfft::get_evaluation_domain<FieldT>(constraint_system.num_constraints() + constraint_system.num_inputs() + 1);
        const FieldT u = domain->evaluate_one_lagrange_polynomials(FFT_evaluation_point, index);
        At -= u * last_polynomial_coefficient;
        At += u * new_polynomial_coefficient; 
        libff::leave_block("Call to r1cs_to_qap_instance_map_with_evaluation_At");
        return At;

    }

    FieldT r1cs_to_qap_instance_map_with_evaluation_Zt(FieldT Zt, FieldT At_save, FieldT new_At, 
                                                            FieldT rA, FieldT beta)
    {
        libff::enter_block("Call to r1cs_to_qap_instance_map_with_evaluation_Zt");
        Zt -= At_save * rA * beta;
        Zt += new_At  * rA * beta; 
        libff::leave_block("Call to r1cs_to_qap_instance_map_with_evaluation_Zt");
        return Zt;

    }

    r1cs_ppzksnark_keypair<ppT> update_proving_key_compilation(libff::Fr<ppT> coef_save,  uint64_t coef_to_update,
                        uint64_t coef_index, 
                        libff::Fr<ppT> FFT_evaluation_point, r1cs_ppzksnark_keypair<ppT> ref_keypair){

        //Change At for save_constraint_a_d_3_j_b_terms
        Fr<ppT> At_save = random_container.At_save;
        random_container.At_save = r1cs_to_qap_instance_map_with_evaluation_At(FFT_evaluation_point, 
                            random_container.At_save, this->poly[coef_to_update], 
                            this->save_constraint_a_d_3_j_b_terms, coef_index);
        Fr<ppT> At_save_index_0 = random_container.At_save;
        random_container.Kt_save = random_container.Kt_save - At_save * random_container.rA * 
                    random_container.beta + random_container.At_save  * random_container.rA * 
                    random_container.beta;

        ref_keypair.pk.K_query[0] = random_container.Kt_save * libff::G1<ppT>::one();

        libff::G1<ppT> encoded_IC_base = (random_container.rA * At_save_index_0) * libff::G1<ppT>::one();
        
        ref_keypair.vk.encoded_IC_query.first = encoded_IC_base;
        return r1cs_ppzksnark_keypair<ppT>(std::move(ref_keypair.pk), std::move(ref_keypair.vk));
    }

    r1cs_constraint_system<FieldT> get_constraint_system(){
        return this->constraint_system;
    }

    void set_random_container(random_container_key<ppT> random_container){
        this->random_container = random_container;
    }

    random_container_key<ppT> get_random_container(){
        return this->random_container;
    }

    void clear_polynomial(){
        this->poly.clear();
    }

    return_container_key_generator_for_update<ppT> r1cs_ppzksnark_key_generator_for_update()
    {
        libff::enter_block("Call to r1cs_ppzksnark_generator");
        const r1cs_ppzksnark_constraint_system<ppT> &cs = (*this->protoboard_for_poly).get_constraint_system();
        /* make the B_query "lighter" if possible */
        r1cs_ppzksnark_constraint_system<ppT> cs_copy(cs);
        cs_copy.swap_AB_if_beneficial();

        /* draw random element at which the QAP is evaluated */
        const  libff::Fr<ppT> t = libff::Fr<ppT>::random_element();

        qap_instance_evaluation<libff::Fr<ppT> > qap_inst = r1cs_to_qap_instance_map_with_evaluation(cs_copy, t);

        libff::print_indent(); printf("* QAP number of variables: %zu\n", qap_inst.num_variables());
        libff::print_indent(); printf("* QAP pre degree: %zu\n", cs_copy.constraints.size());
        libff::print_indent(); printf("* QAP degree: %zu\n", qap_inst.degree());
        libff::print_indent(); printf("* QAP number of input variables: %zu\n", qap_inst.num_inputs());

        libff::enter_block("Compute query densities");
        size_t non_zero_At = 0, non_zero_Bt = 0, non_zero_Ct = 0, non_zero_Ht = 0;
        for (size_t i = 0; i < qap_inst.num_variables()+1; ++i)
        {
            if (!qap_inst.At[i].is_zero())
            {
                ++non_zero_At;
            }
            if (!qap_inst.Bt[i].is_zero())
            {
                ++non_zero_Bt;
            }
            if (!qap_inst.Ct[i].is_zero())
            {
                ++non_zero_Ct;
            }
        }
        for (size_t i = 0; i < qap_inst.degree()+1; ++i)
        {
            if (!qap_inst.Ht[i].is_zero())
            {
                ++non_zero_Ht;
            }
        }
        libff::leave_block("Compute query densities");

        libff::Fr_vector<ppT> At = std::move(qap_inst.At); // qap_inst.At is now in unspecified state, but we do not use it later
        libff::Fr_vector<ppT> Bt = std::move(qap_inst.Bt); // qap_inst.Bt is now in unspecified state, but we do not use it later
        libff::Fr_vector<ppT> Ct = std::move(qap_inst.Ct); // qap_inst.Ct is now in unspecified state, but we do not use it later
        libff::Fr_vector<ppT> Ht = std::move(qap_inst.Ht); // qap_inst.Ht is now in unspecified state, but we do not use it later

        /* append Zt to At,Bt,Ct with */
        At.emplace_back(qap_inst.Zt);
        Bt.emplace_back(qap_inst.Zt);
        Ct.emplace_back(qap_inst.Zt);
        
        Fr<ppT> At_save = At[2];
        Fr<ppT> Bt_save = Bt[2];

        const  libff::Fr<ppT> alphaA = libff::Fr<ppT>::random_element(),
            alphaB = libff::Fr<ppT>::random_element(),
            alphaC = libff::Fr<ppT>::random_element(),
            rA = libff::Fr<ppT>::random_element(),
            rB = libff::Fr<ppT>::random_element(),
            beta = libff::Fr<ppT>::random_element(),
            gamma = libff::Fr<ppT>::random_element();
        const libff::Fr<ppT>  rC = rA * rB;

        // consrtuct the same-coefficient-check query (must happen before zeroing out the prefix of At)
        libff::Fr_vector<ppT> Kt;
        Kt.reserve(qap_inst.num_variables()+4);
        for (size_t i = 0; i < qap_inst.num_variables()+1; ++i)
        {
            Kt.emplace_back( beta * (rA * At[i] + rB * Bt[i] + rC * Ct[i] ) );
        }
        Kt.emplace_back(beta * rA * qap_inst.Zt);
        Kt.emplace_back(beta * rB * qap_inst.Zt);
        Kt.emplace_back(beta * rC * qap_inst.Zt);

        /* zero out prefix of At and stick it into IC coefficients */
        libff::Fr_vector<ppT> IC_coefficients;
        IC_coefficients.reserve(qap_inst.num_inputs() + 1);
        for (size_t i = 0; i < qap_inst.num_inputs() + 1; ++i)
        {
            IC_coefficients.emplace_back(At[i]);
            assert(!IC_coefficients[i].is_zero());
            At[i] = libff::Fr<ppT>::zero();
        }

        const size_t g1_exp_count = 2*(non_zero_At - qap_inst.num_inputs() + non_zero_Ct) + non_zero_Bt + non_zero_Ht + Kt.size();
        const size_t g2_exp_count = non_zero_Bt;

        size_t g1_window = libff::get_exp_window_size<libff::G1<ppT> >(g1_exp_count);
        size_t g2_window = libff::get_exp_window_size<libff::G2<ppT> >(g2_exp_count);
        libff::print_indent(); printf("* G1 window: %zu\n", g1_window);
        libff::print_indent(); printf("* G2 window: %zu\n", g2_window);

    #ifdef MULTICORE
        const size_t chunks = omp_get_max_threads(); // to override, set OMP_NUM_THREADS env var or call omp_set_num_threads()
    #else
        const size_t chunks = 1;
    #endif

        libff::enter_block("Generating G1 multiexp table");
        libff::window_table<libff::G1<ppT> > g1_table = get_window_table(libff::Fr<ppT>::size_in_bits(), g1_window, libff::G1<ppT>::one());
        libff::leave_block("Generating G1 multiexp table");

        libff::enter_block("Generating G2 multiexp table");
        libff::window_table<libff::G2<ppT> > g2_table = get_window_table(libff::Fr<ppT>::size_in_bits(), g2_window, libff::G2<ppT>::one());
        libff::leave_block("Generating G2 multiexp table");

        libff::enter_block("Generate R1CS proving key");

        libff::enter_block("Generate knowledge commitments");
        libff::enter_block("Compute the A-query", false);
        knowledge_commitment_vector<libff::G1<ppT>, libff::G1<ppT> > A_query = kc_batch_exp(libff::Fr<ppT>::size_in_bits(), g1_window, g1_window, g1_table, g1_table, rA, rA*alphaA, At, chunks);
        libff::leave_block("Compute the A-query", false);

        libff::enter_block("Compute the B-query", false);
        knowledge_commitment_vector<libff::G2<ppT>, libff::G1<ppT> > B_query = kc_batch_exp(libff::Fr<ppT>::size_in_bits(), g2_window, g1_window, g2_table, g1_table, rB, rB*alphaB, Bt, chunks);
        libff::leave_block("Compute the B-query", false);

        libff::enter_block("Compute the C-query", false);
        knowledge_commitment_vector<libff::G1<ppT>, libff::G1<ppT> > C_query = kc_batch_exp(libff::Fr<ppT>::size_in_bits(), g1_window, g1_window, g1_table, g1_table, rC, rC*alphaC, Ct, chunks);
        libff::leave_block("Compute the C-query", false);

        libff::enter_block("Compute the H-query", false);
        libff::G1_vector<ppT> H_query = batch_exp(libff::Fr<ppT>::size_in_bits(), g1_window, g1_table, Ht);
    #ifdef USE_MIXED_ADDITION
        libff::batch_to_special<libff::G1<ppT> >(H_query);
    #endif
        libff::leave_block("Compute the H-query", false);

        libff::enter_block("Compute the K-query", false);
        libff::G1_vector<ppT> K_query = batch_exp(libff::Fr<ppT>::size_in_bits(), g1_window, g1_table, Kt);
    #ifdef USE_MIXED_ADDITION
        libff::batch_to_special<libff::G1<ppT> >(K_query);
    #endif
        libff::leave_block("Compute the K-query", false);

        libff::leave_block("Generate knowledge commitments");

        libff::leave_block("Generate R1CS proving key");

        libff::enter_block("Generate R1CS verification key");
        libff::G2<ppT> alphaA_g2 = alphaA * libff::G2<ppT>::one();
        libff::G1<ppT> alphaB_g1 = alphaB * libff::G1<ppT>::one();
        libff::G2<ppT> alphaC_g2 = alphaC * libff::G2<ppT>::one();
        libff::G2<ppT> gamma_g2 = gamma * libff::G2<ppT>::one();
        libff::G1<ppT> gamma_beta_g1 = (gamma * beta) * libff::G1<ppT>::one();
        libff::G2<ppT> gamma_beta_g2 = (gamma * beta) * libff::G2<ppT>::one();
        libff::G2<ppT> rC_Z_g2 = (rC * qap_inst.Zt) * libff::G2<ppT>::one();

        libff::enter_block("Encode IC query for R1CS verification key");
        libff::G1<ppT> encoded_IC_base = (rA * IC_coefficients[0]) * libff::G1<ppT>::one();
        libff::Fr_vector<ppT> multiplied_IC_coefficients;
        multiplied_IC_coefficients.reserve(qap_inst.num_inputs());
        for (size_t i = 1; i < qap_inst.num_inputs() + 1; ++i)
        {
            multiplied_IC_coefficients.emplace_back(rA * IC_coefficients[i]);
        }
        libff::G1_vector<ppT> encoded_IC_values = batch_exp(libff::Fr<ppT>::size_in_bits(), g1_window, g1_table, multiplied_IC_coefficients);

        libff::leave_block("Encode IC query for R1CS verification key");
        libff::leave_block("Generate R1CS verification key");

        libff::leave_block("Call to r1cs_ppzksnark_generator");

        accumulation_vector<libff::G1<ppT> > encoded_IC_query(std::move(encoded_IC_base), std::move(encoded_IC_values));

        r1cs_ppzksnark_verification_key<ppT> vk = r1cs_ppzksnark_verification_key<ppT>(alphaA_g2,
                                                                                    alphaB_g1,
                                                                                    alphaC_g2,
                                                                                    gamma_g2,
                                                                                    gamma_beta_g1,
                                                                                    gamma_beta_g2,
                                                                                    rC_Z_g2,
                                                                                    encoded_IC_query);
        r1cs_ppzksnark_proving_key<ppT> pk = r1cs_ppzksnark_proving_key<ppT>(std::move(A_query),
                                                                            std::move(B_query),
                                                                            std::move(C_query),
                                                                            std::move(H_query),
                                                                            std::move(K_query),
                                                                            std::move(cs_copy));

        random_container_key<ppT> container_random = random_container_key<ppT>(alphaA,
                                                                            alphaB,
                                                                            alphaC,
                                                                            rA,
                                                                            rB,
                                                                            beta,
                                                                            gamma,
                                                                            At_save,
                                                                            Bt_save,
                                                                            Kt[0]);

        pk.print_size();
        vk.print_size();
        return_container_key_generator_for_update<ppT> return_container(t, container_random, r1cs_ppzksnark_keypair<ppT>(std::move(pk), std::move(vk)));
        return return_container;
    }

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
    
};
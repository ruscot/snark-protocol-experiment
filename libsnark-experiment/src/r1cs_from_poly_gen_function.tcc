template<typename FieldT, typename ppT>
R1CS_Polynomial_factory<FieldT, ppT>::R1CS_Polynomial_factory(uint64_t degree, int insert_error_for_test){
    this->polynomial_degree = degree;
    this->insert_error_for_test = insert_error_for_test;
    this->protoboard_for_poly = NULL;
    this->already_changed_one_time = false;
    this->coefficient_one_updated = libff::Fr<ppT>::zero();
    this->coefficient_zero_updated = libff::Fr<ppT>::zero();
}

template<typename FieldT, typename ppT>
void R1CS_Polynomial_factory<FieldT, ppT>::update_coefficient_zero(libff::Fr<ppT> new_zero_coefficient_value, libff::Fr<ppT> last_zero_coefficient_value){
    if(this->insert_error_for_test == 1){
        this->coefficient_zero_updated = new_zero_coefficient_value;
        this->save_coefficient_zero = last_zero_coefficient_value;
    } else {
        this->coefficient_zero_updated += new_zero_coefficient_value;
        this->save_coefficient_zero += last_zero_coefficient_value;
    }
}

template<typename FieldT, typename ppT>
pb_variable<FieldT> R1CS_Polynomial_factory<FieldT, ppT>::get_y_variable(){
    return this->y;
}

template<typename FieldT, typename ppT>
void R1CS_Polynomial_factory<FieldT, ppT>::update_coefficient_one(libff::Fr<ppT> new_one_coefficient_value, libff::Fr<ppT> last_one_coefficient_value){
    if(this->insert_error_for_test == 1){
        this->coefficient_one_updated = new_one_coefficient_value;
        this->save_coefficient_one = last_one_coefficient_value;
    } else {
        this->coefficient_one_updated += new_one_coefficient_value;
        this->save_coefficient_one += last_one_coefficient_value;
    }
}

template<typename FieldT, typename ppT>
void R1CS_Polynomial_factory<FieldT, ppT>::set_x_value(libff::Fr<ppT> x_value){
    this->x_value = x_value;
}

template<typename FieldT, typename ppT>
uint64_t R1CS_Polynomial_factory<FieldT, ppT>::get_polynomial_degree(){
    return this->polynomial_degree;
}

template<typename FieldT, typename ppT>
libff::Fr<ppT> R1CS_Polynomial_factory<FieldT, ppT>::protocol_result_updated(libff::Fr<ppT> current_result_of_our_protocol){
    if(this->coefficient_one_updated != libff::Fr<ppT>::zero()){
        current_result_of_our_protocol += this->coefficient_one_updated * this->x_value - this->save_coefficient_one * this->x_value;
    }
    if(this->coefficient_zero_updated != libff::Fr<ppT>::zero()){
        current_result_of_our_protocol += this->coefficient_zero_updated - this->save_coefficient_zero;
    }

    return current_result_of_our_protocol;
}

template<typename FieldT, typename ppT>
vector<libff::Fr<ppT>> R1CS_Polynomial_factory<FieldT, ppT>::get_polynomial(){
    return this->poly;
}

template<typename FieldT, typename ppT>
libff::Fr<ppT> R1CS_Polynomial_factory<FieldT, ppT>::get_polynomial_coefficients(uint64_t coef){
    return this->poly[coef];
}

template<typename FieldT, typename ppT>
pb_variable<FieldT> R1CS_Polynomial_factory<FieldT, ppT>::get_x_variable(){
    return this->x;
}

template<typename FieldT, typename ppT>
void R1CS_Polynomial_factory<FieldT, ppT>::update_polynomial_coefficient(libff::Fr<ppT> new_coef, uint64_t index_of_coefficient){
    this->poly[index_of_coefficient] = new_coef;
}

template<typename FieldT, typename ppT>
void R1CS_Polynomial_factory<FieldT, ppT>::create_random_coefficients_for_polynomial(){
    for(size_t i = 0; i <= this->polynomial_degree; ++i) {
        this->poly.push_back(4);//libff::Fr<ppT>::random_element());
    }
}

template<typename FieldT, typename ppT>
void R1CS_Polynomial_factory<FieldT, ppT>::create_wrong_constraint_horner_method()
{
    libff::enter_block("Create constraint horner method");
    this->y.allocate(*this->protoboard_for_poly, "out");
    this->x.allocate(*this->protoboard_for_poly, "x");

    if(this->polynomial_degree == 0){
        throw std::runtime_error("Polynomial of degree 0, not valid");
    } else {
        pb_variable<FieldT> last_var_1;
        last_var_1.allocate(*this->protoboard_for_poly, "0");
        (*this->protoboard_for_poly).add_r1cs_constraint(r1cs_constraint<FieldT>(this->poly[0] + 0 * this->x, this->x, last_var_1));
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

template<typename FieldT, typename ppT>
void R1CS_Polynomial_factory<FieldT, ppT>::create_constraint_horner_method()
{
    libff::enter_block("Create constraint horner method");
    this->y.allocate(*this->protoboard_for_poly, "out");
    this->x.allocate(*this->protoboard_for_poly, "x");

    if(this->polynomial_degree == 0){
        throw std::runtime_error("Polynomial of degree 0, not valid");
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

template<typename FieldT, typename ppT>
libff::Fr<ppT> R1CS_Polynomial_factory<FieldT, ppT>::evaluation_polynomial_horner() {
    libff::Fr<ppT> x_value = (*this->protoboard_for_poly).auxiliary_input()[0];
    libff::Fr<ppT> res = this->poly[this->polynomial_degree];
    for(uint64_t i = this->polynomial_degree; i > 0; i-=1){
        res = res*x_value + this->poly[i-1];
    }
    return res;
}

template<typename FieldT, typename ppT>
void R1CS_Polynomial_factory<FieldT, ppT>::update_constraint_horner_method(uint64_t index_of_the_coefficient, libff::Fr<ppT> delta)
{
    libff::enter_block("Update constraint horner method");
    uint64_t index_in_the_r1cs = this->polynomial_degree - index_of_the_coefficient;
    FieldT random_k = 6;//libff::Fr<ppT>::random_element();
    r1cs_constraint<FieldT> constraint_a_d_3_j = (*this->protoboard_for_poly).get_specific_constraint_in_r1cs(index_in_the_r1cs);
    random_k.print();
    random_k.inverse().print();
    /*cout << "\n\n////////////////////////////////////////////////////////////////////"<<endl;
    cout << "constraint a  constraint_a_d_3_j"<<endl;
    constraint_a_d_3_j.a.print();
    cout << "constraint b "<<endl;
    constraint_a_d_3_j.b.print();
    cout << "//////////////////////////////////////////////////////////////"<<endl;*/
    if(this->insert_error_for_test == 1){
        this->save_constraint_a_d_3_j_b_terms = constraint_a_d_3_j.b.terms[0].coeff;
        constraint_a_d_3_j.b.terms[0].coeff = random_k;
        this->new_constraint_a_d_3_j_b_terms = constraint_a_d_3_j.b.terms[0].coeff;    
    } else {
        this->save_constraint_a_d_3_j_b_terms = constraint_a_d_3_j.b.terms[0].coeff;
        constraint_a_d_3_j.b.terms[0].coeff *= random_k;
        this->new_constraint_a_d_3_j_b_terms = constraint_a_d_3_j.b.terms[0].coeff;
    }
    cout << "\n\n////////////////////////////////////////////////////////////////////"<<endl;
    cout << "constraint a constraint_a_d_3_j"<<endl;
    constraint_a_d_3_j.a.print();
    cout << "constraint b "<<endl;
    constraint_a_d_3_j.b.print();
    cout << "//////////////////////////////////////////////////////////////"<<endl;
    r1cs_constraint<FieldT> constraint_a_d_4_j = (*this->protoboard_for_poly).get_specific_constraint_in_r1cs(index_in_the_r1cs + 1);
    r1cs_constraint<FieldT> constraint_a_d_5_j = (*this->protoboard_for_poly).get_specific_constraint_in_r1cs(index_in_the_r1cs + 2);
    this->save_constraint_a_d_4_j_a_terms = constraint_a_d_4_j.a.terms[1].coeff;

    if(constraint_a_d_5_j.b.terms[0].coeff*constraint_a_d_4_j.b.terms[0].coeff != libff::Fr<ppT>::one()){
        constraint_a_d_4_j.a.terms[1].coeff = constraint_a_d_4_j.a.terms[1].coeff * random_k +
                                    delta * random_k * constraint_a_d_4_j.b.terms[0].coeff.inverse() *constraint_a_d_5_j.b.terms[0].coeff.inverse();
    } else {
        constraint_a_d_4_j.a.terms[1].coeff = constraint_a_d_4_j.a.terms[1].coeff * random_k +
                                    delta * random_k * constraint_a_d_4_j.b.terms[0].coeff.inverse() * 
                                    constraint_a_d_5_j.b.terms[0].coeff.inverse();
    }

    this->new_constraint_a_d_4_j_a_terms = constraint_a_d_4_j.a.terms[1].coeff;

    this->save_constraint_a_d_4_j_b_terms = constraint_a_d_4_j.b.terms[0].coeff;
    constraint_a_d_4_j.b.terms[0].coeff *= random_k.inverse();
    this->new_constraint_a_d_4_j_b_terms = constraint_a_d_4_j.b.terms[0].coeff;

    cout << "////////////////////////////////////////////////////////////////////"<<endl;
    cout << "constraint a  constraint_a_d_4_j"<<endl;
    constraint_a_d_4_j.a.print();
    cout << "constraint b "<<endl;
    constraint_a_d_4_j.b.print();
    cout << "//////////////////////////////////////////////////////////////"<<endl;
    
    this->save_constraint_a_d_5_j_a_terms = constraint_a_d_5_j.a.terms[1].coeff;
    /*cout << "////////////////////////////////////////////////////////////////////"<<endl;
    cout << "constraint a  constraint_a_d_5_j"<<endl;
    constraint_a_d_5_j.a.print();
    cout << "constraint b "<<endl;
    constraint_a_d_5_j.b.print();
    cout << "//////////////////////////////////////////////////////////////"<<endl;*/
    constraint_a_d_5_j.a.terms[1].coeff += (libff::Fr<ppT>::one() - random_k.inverse()) * constraint_a_d_4_j.a.terms[0].coeff * 
                                                this->save_constraint_a_d_4_j_b_terms ;
    
    cout << "////////////////////////////////////////////////////////////////////"<<endl;
    cout << "constraint a  constraint_a_d_5_j"<<endl;
    constraint_a_d_5_j.a.print();
    cout << "constraint b "<<endl;
    constraint_a_d_5_j.b.print();
    cout << "//////////////////////////////////////////////////////////////"<<endl;
    this->new_constraint_a_d_5_j_a_terms = constraint_a_d_5_j.a.terms[1].coeff;

    
    (*this->protoboard_for_poly).protoboard_update_r1cs_constraint(constraint_a_d_3_j, index_in_the_r1cs);
    (*this->protoboard_for_poly).protoboard_update_r1cs_constraint(constraint_a_d_4_j, index_in_the_r1cs + 1);
    (*this->protoboard_for_poly).protoboard_update_r1cs_constraint(constraint_a_d_5_j, index_in_the_r1cs + 2);
    
    libff::leave_block("Update constraint horner method");
}

template<typename FieldT, typename ppT>
void R1CS_Polynomial_factory<FieldT, ppT>::set_protoboard(protoboard<FieldT>* protoboard_for_poly){
    this->protoboard_for_poly = protoboard_for_poly;
}

template<typename FieldT, typename ppT>
protoboard<FieldT> R1CS_Polynomial_factory<FieldT, ppT>::get_protoboard(){
    return (*this->protoboard_for_poly);
}

template<typename FieldT, typename ppT>
void R1CS_Polynomial_factory<FieldT, ppT>::set_constraint_system(r1cs_constraint_system<FieldT> constraint_system){
    this->constraint_system = constraint_system;
}

template<typename FieldT, typename ppT>
FieldT R1CS_Polynomial_factory<FieldT, ppT>::r1cs_to_qap_instance_map_with_evaluation_At(const FieldT &FFT_evaluation_point, FieldT At, 
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

template<typename FieldT, typename ppT>
FieldT R1CS_Polynomial_factory<FieldT, ppT>::r1cs_to_qap_instance_map_with_evaluation_Zt(FieldT Zt, FieldT At_save, FieldT new_At, 
                                                        FieldT rA, FieldT beta)
{
    libff::enter_block("Call to r1cs_to_qap_instance_map_with_evaluation_Zt");
    Zt -= At_save * rA * beta;
    Zt += new_At  * rA * beta; 
    libff::leave_block("Call to r1cs_to_qap_instance_map_with_evaluation_Zt");
    return Zt;

}

template<typename FieldT, typename ppT>
r1cs_ppzksnark_keypair<ppT> R1CS_Polynomial_factory<FieldT, ppT>::update_proving_key_compilation(uint64_t coef_index, 
                    libff::Fr<ppT> FFT_evaluation_point){
    Fr<ppT> At_save = random_container.At_save;
    Fr<ppT> Bt_save = random_container.Bt_save;
    
    if(this->insert_error_for_test != 2){
        //Change At for constraint_a_d_4_j
        random_container.At_save = r1cs_to_qap_instance_map_with_evaluation_At(FFT_evaluation_point, 
                random_container.At_save, this->new_constraint_a_d_4_j_a_terms, 
                this->save_constraint_a_d_4_j_a_terms, coef_index + 1);
    }
    if(!this->already_changed_one_time){
        //Change At for constraint_a_d_5_j
        random_container.At_save = r1cs_to_qap_instance_map_with_evaluation_At(FFT_evaluation_point, 
                    random_container.At_save, this->new_constraint_a_d_5_j_a_terms, 
                    this->save_constraint_a_d_5_j_a_terms, coef_index + 2);
        (*this->current_key_pair).pk.A_query.values.emplace((*this->current_key_pair).pk.A_query.values.begin(), knowledge_commitment<libff::G1<ppT>, libff::G1<ppT>>(
                    random_container.At_save * random_container.rA * libff::G1<ppT>::one(),
                    random_container.At_save * random_container.rA*random_container.alphaA * libff::G1<ppT>::one()));
        (*this->current_key_pair).pk.A_query.indices.emplace((*this->current_key_pair).pk.A_query.indices.begin(), 2);

        random_container.Bt_save = r1cs_to_qap_instance_map_with_evaluation_At(FFT_evaluation_point, 
                    random_container.Bt_save, this->new_constraint_a_d_3_j_b_terms, 
                    this->save_constraint_a_d_3_j_b_terms, coef_index);
        random_container.Bt_save = r1cs_to_qap_instance_map_with_evaluation_At(FFT_evaluation_point, 
                    random_container.Bt_save, this->new_constraint_a_d_4_j_b_terms, 
                    this->save_constraint_a_d_4_j_b_terms, coef_index + 1);
        (*this->current_key_pair).pk.B_query.values[1] = knowledge_commitment<libff::G2<ppT>, libff::G1<ppT>>(
                    random_container.Bt_save * random_container.rB * libff::G2<ppT>::one() ,
                    random_container.Bt_save * random_container.rB * random_container.alphaB * libff::G1<ppT>::one() );

        random_container.Kt_save = random_container.Kt_save - At_save * random_container.rA * 
                    random_container.beta + random_container.At_save  * random_container.rA * 
                    random_container.beta - Bt_save * random_container.rB * 
                    random_container.beta + random_container.Bt_save  * random_container.rB * 
                    random_container.beta;
        
        (*this->current_key_pair).pk.K_query[2] = random_container.Kt_save * libff::G1<ppT>::one();
        this->already_changed_one_time = true;
    } else {
        random_container.At_save = r1cs_to_qap_instance_map_with_evaluation_At(FFT_evaluation_point, 
                    random_container.At_save, this->new_constraint_a_d_5_j_a_terms, 
                    this->save_constraint_a_d_5_j_a_terms, coef_index + 2);
        (*this->current_key_pair).pk.A_query.values[0] = knowledge_commitment<libff::G1<ppT>, libff::G1<ppT>>(
                    random_container.At_save * random_container.rA * libff::G1<ppT>::one(),
                    random_container.At_save * random_container.rA*random_container.alphaA * libff::G1<ppT>::one());

        random_container.Bt_save = r1cs_to_qap_instance_map_with_evaluation_At(FFT_evaluation_point, 
                    random_container.Bt_save, this->new_constraint_a_d_3_j_b_terms, 
                    this->save_constraint_a_d_3_j_b_terms, coef_index);
        random_container.Bt_save = r1cs_to_qap_instance_map_with_evaluation_At(FFT_evaluation_point, 
                    random_container.Bt_save, this->new_constraint_a_d_4_j_b_terms, 
                    this->save_constraint_a_d_4_j_b_terms, coef_index + 1);
        (*this->current_key_pair).pk.B_query.values[1] = knowledge_commitment<libff::G2<ppT>, libff::G1<ppT>>(
                    random_container.Bt_save * random_container.rB * libff::G2<ppT>::one() ,
                    random_container.Bt_save * random_container.rB * random_container.alphaB * libff::G1<ppT>::one() );

        random_container.Kt_save = random_container.Kt_save - At_save * random_container.rA * 
                    random_container.beta + random_container.At_save  * random_container.rA * 
                    random_container.beta - Bt_save * random_container.rB * 
                    random_container.beta + random_container.Bt_save  * random_container.rB * 
                    random_container.beta;
        
        (*this->current_key_pair).pk.K_query[2] = random_container.Kt_save * libff::G1<ppT>::one();
    }
    
    return (*this->current_key_pair);
}

template<typename FieldT, typename ppT>
r1cs_constraint_system<FieldT> R1CS_Polynomial_factory<FieldT, ppT>::get_constraint_system(){
    return this->constraint_system;
}

template<typename FieldT, typename ppT>
void R1CS_Polynomial_factory<FieldT, ppT>::set_random_container(random_container_key<ppT> random_container){
    this->random_container = random_container;
}

template<typename FieldT, typename ppT>
random_container_key<ppT> R1CS_Polynomial_factory<FieldT, ppT>::get_random_container(){
    return this->random_container;
}

template<typename FieldT, typename ppT>
void R1CS_Polynomial_factory<FieldT, ppT>::clear_polynomial(){
    this->poly.clear();
}

template<typename FieldT, typename ppT>
return_container_key_generator_for_update<ppT> R1CS_Polynomial_factory<FieldT, ppT>::r1cs_ppzksnark_key_generator_for_update()
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
                                                                        Kt[2]);

    pk.print_size();
    vk.print_size();
    return_container_key_generator_for_update<ppT> return_container(t, container_random, r1cs_ppzksnark_keypair<ppT>(std::move(pk), std::move(vk)));
    return return_container;
}

template<typename FieldT, typename ppT>
void R1CS_Polynomial_factory<FieldT, ppT>::set_current_key_pair(r1cs_ppzksnark_keypair<ppT> *key_pair){
    this->current_key_pair = key_pair;
}

template<typename FieldT, typename ppT>
r1cs_ppzksnark_keypair<ppT>* R1CS_Polynomial_factory<FieldT, ppT>::get_current_key_pair(){
    return this->current_key_pair;
}

template<typename FieldT, typename ppT>
void R1CS_Polynomial_factory<FieldT, ppT>::update_keypair_constraint_system(const r1cs_constraint_system<FieldT> constraint_system_update){
    this->current_key_pair->pk.constraint_system = constraint_system_update;
}

template<typename FieldT, typename ppT>
r1cs_ppzksnark_proving_key<ppT> R1CS_Polynomial_factory<FieldT, ppT>::get_proving_key(){
    return this->current_key_pair->pk;
}

template<typename FieldT, typename ppT>
r1cs_ppzksnark_verification_key<ppT> R1CS_Polynomial_factory<FieldT, ppT>::get_verification_key(){
    return this->current_key_pair->vk;
}
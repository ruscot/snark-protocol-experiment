#ifndef R1CS_UPDATE_KEYS_CPP_
#define R1CS_UPDATE_KEYS_CPP_

#include "r1cs_update_keys.hpp"

namespace libsnark {

template<typename ppT>
class random_container_key {
public:
    libff::Fr<ppT> alphaA;
    libff::Fr<ppT> alphaB;
    libff::Fr<ppT> alphaC;
    libff::Fr<ppT> rA;
    libff::Fr<ppT> rB;
    libff::Fr<ppT> beta;
    libff::Fr<ppT> gamma;
    Fr<ppT> At_save;
    Fr<ppT> Bt_save;
    Fr<ppT> Kt_save;
    random_container_key(libff::Fr<ppT> alphaA,
        libff::Fr<ppT> alphaB,
        libff::Fr<ppT> alphaC,
        libff::Fr<ppT> rA,
        libff::Fr<ppT> rB,
        libff::Fr<ppT> beta,
        libff::Fr<ppT> gamma,
        Fr<ppT> At_save,
        Fr<ppT> Bt_save,
        Fr<ppT> Kt_save) :
        alphaA(alphaA),
        alphaB(alphaB),
        alphaC(alphaC),
        rA(rA),
        rB(rB),
        beta(beta),
        gamma(gamma),
        At_save(At_save),
        Bt_save(Bt_save),
        Kt_save(Kt_save)
    {};
    random_container_key()
    {};
};

template<typename ppT>
class return_container_key_generator_for_update {
private: 
    Fr<ppT> FFT_evaluation_point;
    random_container_key<ppT> random_container;
    r1cs_ppzksnark_keypair<ppT> key_pair;
public:
    return_container_key_generator_for_update(Fr<ppT> FFT_evaluation_point,
        random_container_key<ppT> random_container,
        r1cs_ppzksnark_keypair<ppT> key_pair):
        FFT_evaluation_point(FFT_evaluation_point),
        random_container(random_container),
        key_pair(key_pair)
    {};
    Fr<ppT> get_FFT_evaluation_point(){
        return FFT_evaluation_point;
    }
    random_container_key<ppT> get_random_container(){
        return random_container;
    }
    r1cs_ppzksnark_keypair<ppT> get_key_pair(){
        return key_pair;
    }
};

template <typename ppT>
r1cs_ppzksnark_keypair<ppT> r1cs_ppzksnark_update(const r1cs_ppzksnark_constraint_system<ppT> &cs/*, qap_instance_evaluation<libff::Fr<ppT>> qap_inst*/,
        libff::Fr<ppT> coef, uint64_t coef_index, uint64_t degree, r1cs_ppzksnark_keypair<ppT> keypair)
{
    return keypair;
}

template <typename ppT>
r1cs_ppzksnark_keypair<ppT> r1cs_ppzksnark_update_proving_key_test(const r1cs_ppzksnark_keypair<ppT> ref_keypair){
    return ref_keypair;

}

template <typename ppT>
std::tuple<r1cs_ppzksnark_keypair<ppT>, libff::Fr_vector<ppT>> r1cs_ppzksnark_generator_with_FFT_evaluation_point_and_random_values(const r1cs_ppzksnark_constraint_system<ppT> &cs, const libff::Fr<ppT> FFT_evaluation_point, 
                    random_container_key<ppT> random_container)
{
    libff::enter_block("Call to r1cs_ppzksnark_generator_with_t");

    /* make the B_query "lighter" if possible */
    r1cs_ppzksnark_constraint_system<ppT> cs_copy(cs);
    cs_copy.swap_AB_if_beneficial();

    qap_instance_evaluation<libff::Fr<ppT> > qap_inst = r1cs_to_qap_instance_map_with_evaluation(cs_copy, FFT_evaluation_point);

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

    const  libff::Fr<ppT> alphaA = random_container.alphaA,
        alphaB = random_container.alphaB,
        alphaC = random_container.alphaC,
        rA = random_container.rA,
        rB = random_container.rB,
        beta = random_container.beta,
        gamma = random_container.gamma;
    const libff::Fr<ppT> rC = rA * rB;

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
    libff::Fr_vector<ppT> At_save = At;
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

    libff::leave_block("Call to r1cs_ppzksnark_generator_with_t");

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
    std::tuple<r1cs_ppzksnark_keypair<ppT>, libff::Fr_vector<ppT>> foo (r1cs_ppzksnark_keypair<ppT>(std::move(pk), std::move(vk)), At_save);
    return foo;
}

template <typename ppT>
void compare_keypair(r1cs_ppzksnark_keypair<ppT> ref_keypair, r1cs_ppzksnark_keypair<ppT> new_keypair){
    libff::enter_block("Call to compare_keypair");

    libff::enter_block("Compare verification key");

    cout << "AlphaA_g2................................";
    if(ref_keypair.vk.alphaA_g2 == new_keypair.vk.alphaA_g2){
        cout << "OK" << endl;
    } else {
        cout << "NOT OK" << endl;
    }

    cout << "alphaB_g1................................";
    if(ref_keypair.vk.alphaB_g1 == new_keypair.vk.alphaB_g1){
        cout << "OK" << endl;
    } else {
        cout << "NOT OK" << endl;
    }

    cout << "alphaC_g2................................";
    if(ref_keypair.vk.alphaC_g2 == new_keypair.vk.alphaC_g2){
        cout << "OK" << endl;
    } else {
        cout << "NOT OK" << endl;
    }

    cout << "gamma_g2.................................";
    if(ref_keypair.vk.gamma_g2 == new_keypair.vk.gamma_g2){
        cout << "OK" << endl;
    } else {
        cout << "NOT OK" << endl;
    }

    cout << "gamma_beta_g1............................";
    if(ref_keypair.vk.gamma_beta_g1 == new_keypair.vk.gamma_beta_g1){
        cout << "OK" << endl;
    } else {
        cout << "NOT OK" << endl;
    }

    cout << "gamma_beta_g2............................";
    if(ref_keypair.vk.gamma_beta_g2 == new_keypair.vk.gamma_beta_g2){
        cout << "OK" << endl;
    } else {
        cout << "NOT OK" << endl;
    }

    cout << "rC_Z_g2..................................";
    if(ref_keypair.vk.rC_Z_g2 == new_keypair.vk.rC_Z_g2){
        cout << "OK" << endl;
    } else {
        cout << "NOT OK" << endl;
    }
    cout << endl;

    
    string text = "OK";
    if(ref_keypair.vk.encoded_IC_query.first == new_keypair.vk.encoded_IC_query.first){
        if(ref_keypair.vk.encoded_IC_query.rest.domain_size_ != new_keypair.vk.encoded_IC_query.rest.domain_size_){
            cout << "encoded_IC_query_domain_size_............NOT OK"<<endl;
        } else {
            cout << "encoded_IC_query_domain_size_............OK"<<endl;
        }

        if(ref_keypair.vk.encoded_IC_query.rest.values.size() != new_keypair.vk.encoded_IC_query.rest.values.size()){
            cout << "encoded_IC_query_values_size.............NOT OK"<<endl;
        } else {
            cout << "encoded_IC_query_values_size.............OK"<<endl;
        }

        for(unsigned int i = 0; i < ref_keypair.vk.encoded_IC_query.rest.indices.size() && i < new_keypair.vk.encoded_IC_query.rest.indices.size(); i++){
            if(ref_keypair.vk.encoded_IC_query.rest.indices[i] != new_keypair.vk.encoded_IC_query.rest.indices[i]) {
                cout << "index " << i  << " not ok for encoded_IC_query "<< endl;
                text = "NOT OK";
            }
        }
        cout << "encoded_IC_query_indices.................";
        cout << text << endl;

        text = "OK";
        for(unsigned int i = 0; i < ref_keypair.vk.encoded_IC_query.rest.values.size() && i < new_keypair.vk.encoded_IC_query.rest.values.size(); i++){
            if(ref_keypair.vk.encoded_IC_query.rest.values[i] != new_keypair.vk.encoded_IC_query.rest.values[i]) {
                cout << "index " << i  << " not ok for encoded_IC_query "<< endl;
                text = "NOT OK";
            }
        }
        cout << "encoded_IC_query_values..................";
        cout << text << endl;
        
    } else {
        cout << "encoded_IC_query_first...................";
        cout << "NOT OK" << endl;
    }

    libff::leave_block("Compare verification key");

    libff::enter_block("Compare proving key");

    if(ref_keypair.pk.A_query.domain_size_ != new_keypair.pk.A_query.domain_size_){
        cout << "A_query_domain_size_.....................NOT OK"<<endl;
    } else {
        cout << "A_query_domain_size_.....................OK"<<endl;
    }

    if(ref_keypair.pk.A_query.values.size() != new_keypair.pk.A_query.values.size()){
        cout << "A_query_values_size......................NOT OK"<<endl;
    } else {
        cout << "A_query_values_size......................OK"<<endl;
    }
    text = "OK";
    for(unsigned int i = 0; i < ref_keypair.pk.A_query.indices.size() && i < new_keypair.pk.A_query.indices.size(); i++){
        if(ref_keypair.pk.A_query.indices[i] != new_keypair.pk.A_query.indices[i]) {
            cout << "index " << i  << " not ok for A_query "<< endl;
            text = "NOT OK";
        }
    }
    cout << "A_query_indices..........................";
    cout << text << endl;

    text = "OK";
    for(unsigned int i = 0; i < ref_keypair.pk.A_query.values.size() && i < new_keypair.pk.A_query.values.size(); i++){
        if(ref_keypair.pk.A_query.values[i] != new_keypair.pk.A_query.values[i]) {
            cout << "index " << i  << " not ok for A_query "<< endl;
            text = "NOT OK";
        }
    }
    cout << "A_query_values...........................";
    cout << text << endl;  
    cout << endl; 

    
    if(ref_keypair.pk.B_query.domain_size_ != new_keypair.pk.B_query.domain_size_){
        cout << "B_query_domain_size_.....................NOT OK"<<endl;
    } else {
        cout << "B_query_domain_size_.....................OK"<<endl;
    }

    if(ref_keypair.pk.B_query.values.size() != new_keypair.pk.B_query.values.size()){
        cout << "B_query_values_size......................NOT OK"<<endl;
    } else {
        cout << "B_query_values_size......................OK"<<endl;
    }
    text="OK";
    for(unsigned int i = 0; i < ref_keypair.pk.B_query.indices.size() && i < new_keypair.pk.B_query.indices.size(); i++){
        if(ref_keypair.pk.B_query.indices[i] != new_keypair.pk.B_query.indices[i]) {
            cout << "index " << i  << " not ok for B_query "<< endl;
            text = "NOT OK";
        }
    }
    cout << "B_query_indices..........................";
    cout << text << endl;

    text = "OK";
    for(unsigned int i = 0; i < ref_keypair.pk.B_query.values.size() && i < new_keypair.pk.B_query.values.size(); i++){
        if(ref_keypair.pk.B_query.values[i] != new_keypair.pk.B_query.values[i]) {
            cout << "index " << i  << " not ok for B_query "<< endl;
            text = "NOT OK";
        }
    }
    cout << "B_query_values...........................";
    cout << text << endl; 
    cout << endl; 


    if(ref_keypair.pk.C_query.domain_size_ != new_keypair.pk.C_query.domain_size_){
        cout << "C_query_domain_size_.....................NOT OK"<<endl;
    } else {
        cout << "C_query_domain_size_.....................OK"<<endl;
    }

    if(ref_keypair.pk.C_query.values.size() != new_keypair.pk.C_query.values.size()){
        cout << "C_query_values_size......................NOT OK"<<endl;
    } else {
        cout << "C_query_values_size......................OK"<<endl;
    }
    text = "OK";
    for(unsigned int i = 0; i < ref_keypair.pk.C_query.indices.size() && i < new_keypair.pk.C_query.indices.size(); i++){
        if(ref_keypair.pk.C_query.indices[i] != new_keypair.pk.C_query.indices[i]) {
            cout << "index " << i  << " not ok for C_query "<< endl;
            text = "NOT OK";
        }
    }
    cout << "C_query_indices..........................";
    cout << text << endl;

    text = "OK";
    for(unsigned int i = 0; i < ref_keypair.pk.C_query.values.size() && i < new_keypair.pk.C_query.values.size(); i++){
        if(ref_keypair.pk.C_query.values[i] != new_keypair.pk.C_query.values[i]) {
            cout << "index " << i  << " not ok for C_query "<< endl;
            text = "NOT OK";
        }
    }
    cout << "C_query..................................";
    cout << text << endl; 
    cout << endl;


    if(ref_keypair.pk.H_query.size() != new_keypair.pk.H_query.size()){
        cout << "H_query_size.............................NOT OK"<<endl;
    } else {
        cout << "H_query_size.............................OK"<<endl;
    }

    text = "OK";
    for(unsigned int i = 0; i < ref_keypair.pk.H_query.size() && i < new_keypair.pk.H_query.size(); i++){
        if(ref_keypair.pk.H_query[i] != new_keypair.pk.H_query[i]) {
            cout << "index " << i  << " not ok for C_query "<< endl;
            text = "NOT OK";
        }
    }
    cout << "H_query..................................";
    cout << text << endl; 
    cout << endl; 


    if(ref_keypair.pk.K_query.size() != new_keypair.pk.K_query.size()){
        cout << "K_query_size.............................NOT OK"<<endl;
    } else {
        cout << "K_query_size.............................OK"<<endl;
    }

    text = "OK";
    for(unsigned int i = 0; i < ref_keypair.pk.K_query.size() && i < new_keypair.pk.K_query.size(); i++){
        if(ref_keypair.pk.K_query[i] != new_keypair.pk.K_query[i]) {
            cout << "index " << i  << " not ok for K_query "<< endl;
            text = "NOT OK";
        }
    }
    cout << "K_query..................................";
    cout << text << endl; 

    libff::leave_block("Compare proving key");

    libff::leave_block("Call to compare_keypair");
}
} // libsnark
#endif // R1CS_UPDATE_KEYS_CPP_

#ifndef R1CS_UPDATE_KEYS_CPP_
#define R1CS_UPDATE_KEYS_CPP_

#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <sstream>

#include <libff/algebra/scalar_multiplication/multiexp.hpp>
#include <libff/common/profiling.hpp>
#include <libff/common/utils.hpp>
#include <tuple> 
#ifdef MULTICORE
#include <omp.h>
#endif

#include <libsnark/knowledge_commitment/kc_multiexp.hpp>
#include <libsnark/reductions/r1cs_to_qap/r1cs_to_qap.hpp>

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
    Fr<ppT> Kt_save;
    size_t g1_window;

    random_container_key(libff::Fr<ppT> alphaA,
        libff::Fr<ppT> alphaB,
        libff::Fr<ppT> alphaC,
        libff::Fr<ppT> rA,
        libff::Fr<ppT> rB,
        libff::Fr<ppT> beta,
        libff::Fr<ppT> gamma,
        Fr<ppT> At_save,
        Fr<ppT> Kt_save,
        size_t g1_window) :
        alphaA(alphaA),
        alphaB(alphaB),
        alphaC(alphaC),
        rA(rA),
        rB(rB),
        beta(beta),
        gamma(gamma),
        At_save(At_save),
        Kt_save(Kt_save),
        g1_window(g1_window)
    {};
};

template<typename FieldT>
qap_instance_evaluation<FieldT> r1cs_to_qap_instance_map_with_evaluation55(const r1cs_constraint_system<FieldT> &cs,
                                                                         const FieldT &t)
{
    libff::enter_block("Call to r1cs_to_qap_instance_map_with_evaluation");

    const std::shared_ptr<libfqfft::evaluation_domain<FieldT> > domain = libfqfft::get_evaluation_domain<FieldT>(cs.num_constraints() + cs.num_inputs() + 1);

    std::vector<FieldT> At, Bt, Ct, Ht;

    At.resize(cs.num_variables()+1, FieldT::zero());
    Bt.resize(cs.num_variables()+1, FieldT::zero());
    Ct.resize(cs.num_variables()+1, FieldT::zero());
    Ht.reserve(domain->m+1);

    const FieldT Zt = domain->compute_vanishing_polynomial(t);

    libff::enter_block("Compute evaluations of A, B, C, H at t");
    const std::vector<FieldT> u = domain->evaluate_all_lagrange_polynomials(t);
    /**
     * add and process the constraints
     *     input_i * 0 = 0
     * to ensure soundness of input consistency
     */
    for (size_t i = 0; i <= cs.num_inputs(); ++i)
    {
        At[i] = u[cs.num_constraints() + i];
    }
    /* process all other constraints */
    for (size_t i = 0; i < cs.num_constraints(); ++i)
    {
        for (size_t j = 0; j < cs.constraints[i].a.terms.size(); ++j)
        {
            At[cs.constraints[i].a.terms[j].index] +=
                u[i]*cs.constraints[i].a.terms[j].coeff;
        }

        for (size_t j = 0; j < cs.constraints[i].b.terms.size(); ++j)
        {
            Bt[cs.constraints[i].b.terms[j].index] +=
                u[i]*cs.constraints[i].b.terms[j].coeff;
        }

        for (size_t j = 0; j < cs.constraints[i].c.terms.size(); ++j)
        {
            Ct[cs.constraints[i].c.terms[j].index] +=
                u[i]*cs.constraints[i].c.terms[j].coeff;
        }
    }

    FieldT ti = FieldT::one();
    for (size_t i = 0; i < domain->m+1; ++i)
    {
        Ht.emplace_back(ti);
        ti *= t;
    }
    libff::leave_block("Compute evaluations of A, B, C, H at t");

    libff::leave_block("Call to r1cs_to_qap_instance_map_with_evaluation");

    return qap_instance_evaluation<FieldT>(domain,
                                           cs.num_variables(),
                                           domain->m,
                                           cs.num_inputs(),
                                           t,
                                           std::move(At),
                                           std::move(Bt),
                                           std::move(Ct),
                                           std::move(Ht),
                                           Zt);
}

template <typename ppT>
r1cs_ppzksnark_keypair<ppT> r1cs_ppzksnark_update(const r1cs_ppzksnark_constraint_system<ppT> &cs/*, qap_instance_evaluation<libff::Fr<ppT>> qap_inst*/,
        libff::Fr<ppT> coef, uint64_t coef_index, uint64_t degree, r1cs_ppzksnark_keypair<ppT> keypair)
{
    return keypair;
}

template <typename ppT>
r1cs_ppzksnark_keypair<ppT> r1cs_ppzksnark_update_proving_key(/*const r1cs_ppzksnark_constraint_system<ppT> &cs,libff::Fr<ppT> coef_save, 
                    libff::Fr<ppT> new_coef, uint64_t coef_index, uint64_t degree, 
                    libff::Fr<ppT> t, random_container_key<ppT> random_container,*/ r1cs_ppzksnark_keypair<ppT> ref_keypair)
{
    //proving key
    //knowledge_commitment_vector<libff::G1<ppT>, libff::G1<ppT> > A_query = keypair.pk.A_query; 
    //keypair.pk 
    //return keypair;
    /*Fr<ppT> At_save = random_container.At_save;
    random_container.At_save = r1cs_to_qap_instance_map_with_evaluation_At(cs, t, 
                        random_container.At_save, new_coef, 
                        coef_save, index);
    Fr<ppT> At_save_index_0 = random_container.At_save;
    random_container.Kt_save = r1cs_to_qap_instance_map_with_evaluation_Zt2(cs, t, 
                        random_container.Kt_save, At_save, 
                        random_container.At_save, 0, random_container.rA, random_container.beta);
    libff::window_table<libff::G1<ppT> > g1_table = get_window_table(libff::Fr<ppT>::size_in_bits(), random_container.g1_window, libff::G1<ppT>::one());
    ref_keypair.pk.K_query[0] = batch_exp_monomial(libff::Fr<ppT>::size_in_bits(), random_container.g1_window, g1_table, random_container.Kt_save);*/
    return ref_keypair;
}

template <typename ppT>
r1cs_ppzksnark_keypair<ppT> r1cs_ppzksnark_update_proving_key_test(const r1cs_ppzksnark_keypair<ppT> ref_keypair){
    return ref_keypair;

}

template <typename ppT>
r1cs_ppzksnark_keypair<ppT> r1cs_ppzksnark_generator55(const r1cs_ppzksnark_constraint_system<ppT> &cs,libff::Fr<ppT> coef_save, 
                    libff::Fr<ppT> new_coef, uint64_t coef_index, uint64_t degree, 
                    libff::Fr<ppT> t, random_container_key<ppT> random_container)
{
    libff::enter_block("Call to r1cs_ppzksnark_generator");

    /* make the B_query "lighter" if possible */
    r1cs_ppzksnark_constraint_system<ppT> cs_copy(cs);
    cs_copy.swap_AB_if_beneficial();

    /* draw random element at which the QAP is evaluated */
    //const  libff::Fr<ppT> t = libff::Fr<ppT>::random_element();

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
    //const std::shared_ptr<libfqfft::evaluation_domain<ppT> > domain = libfqfft::get_evaluation_domain<ppT>(cs.num_constraints() + cs.num_inputs() + 1);
    //const std::vector<ppT> u = std::get<1>(tuple)->evaluate_all_lagrange_polynomials(std::get<0>(tuple));
    //cout << "At r1cs_ppzksnark_generator55 " << At <<endl;
    uint64_t index = 0;
    //Fr<ppT> At_save = At[cs.constraints[index].a.terms[0].index];
    if(coef_index == degree){
        cout << "here if not done yet  " << endl;
        index = 0;
        exit(0);
        /*cout << "r1cs constraint b " << endl;
        cout << "coef  " << coef << endl;
        cout << constraint.b.terms[0].coeff << endl;
        constraint.b.terms[0].coeff = coef;
        cout << constraint.b.terms[0].coeff << endl;
        (*pb).protoboard_update_r1cs_constraint(constraint, 0, "");
        At[cs.constraints[0].b.terms[0].index] = r1cs_to_qap_instance_map_with_evaluation2(cs_copy, t, 
                                                    At[cs.constraints[0].b.terms[0].index], new_coef, 
                                                    coef_save, 0);*/
    } else if (coef_index == degree - 1) {
        cout << "here else if " << endl;
        cout << "not check yet" << endl;
        index = 1;
        /*At[cs.constraints[1].a.terms[0].index] = r1cs_to_qap_instance_map_with_evaluation2(cs_copy, t, 
                                                    At[cs.constraints[1].a.terms[0].index], new_coef, 
                                                    coef_save, 1);*/
    } else {
        cout << "here else " << endl;
        index = 2 + (degree - coef_index - 2) * 2 +1;
        cout << "index " << index << endl;
        cout << "At index : " << cs.constraints[index].a.terms[0].index << endl;
        cout << "coef save : " << coef_save << endl;
        cout << "new coef : " << new_coef << endl;
        /*At[cs.constraints[index].a.terms[0].index] = r1cs_to_qap_instance_map_with_evaluation_At(cs_copy, t, 
                        At[cs.constraints[index].a.terms[0].index], new_coef, 
                        coef_save, index);*/
    }
    //cout << "At r1cs_ppzksnark_generator55 after change " << At <<endl;
    

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
        //TODO change here with At
        Kt.emplace_back( beta * (rA * At[i] + rB * Bt[i] + rC * Ct[i] ) );
    }
    Kt.emplace_back(beta * rA * qap_inst.Zt);
    Kt.emplace_back(beta * rB * qap_inst.Zt);
    Kt.emplace_back(beta * rC * qap_inst.Zt);
    //Fr<ppT> At_save = At[cs.constraints[index].a.terms[0].index];
    
    /*At[cs.constraints[index].a.terms[0].index] = r1cs_to_qap_instance_map_with_evaluation_At(cs_copy, t, 
                                                    At[cs.constraints[index].a.terms[0].index], new_coef, 
                                                    coef_save, index);*/
    /*cout << "Arguments value : " << endl;
    cout << "   t " << t << endl;
    cout << "   At[cs.constraints[index].a.terms[0].index] " << At[cs.constraints[index].a.terms[0].index] << endl;
    cout << "   new_coef " << new_coef << endl;
    cout << "   coef_save " << coef_save << endl;
    cout << "   index " << index << endl;
    cout << "   At_save " << At_save << endl;
    
    cout << "At value r1cs_ppzksnark_generator55 to check " << At[cs.constraints[index].a.terms[0].index] << endl;
    Fr<ppT> At_save_index_0 = At[cs.constraints[index].a.terms[0].index];
    Fr<ppT> KT_save = r1cs_to_qap_instance_map_with_evaluation_Zt2(cs_copy, t, 
                                                    Kt[0], At_save, 
                                                    At[cs.constraints[index].a.terms[0].index], 0, rA, beta);
    cout << "Arguments value 2 : " << endl;
    cout << "   t " << t << endl;
    cout << "   KT_save " << KT_save << endl;
    cout << "   At[cs.constraints[index].a.terms[0].index] " << At[cs.constraints[index].a.terms[0].index] << endl;
    cout << "   rA " << rA << endl;
    cout << "   beta " << beta << endl;
    cout << "   At_save " << At_save << endl;*/
    /*Kt[0] = r1cs_to_qap_instance_map_with_evaluation_Zt(cs_copy, t, 
                                                    Kt[0], new_coef, 
                                                    coef_save, 0, rA, beta);*/

    /* zero out prefix of At and stick it into IC coefficients */
    libff::Fr_vector<ppT> IC_coefficients;
    IC_coefficients.reserve(qap_inst.num_inputs() + 1);
    for (size_t i = 0; i < qap_inst.num_inputs() + 1; ++i)
    {
        //TODO change IC coef
        cout << "I for ic coef " << i << endl;
        IC_coefficients.emplace_back(At[i]);
        assert(!IC_coefficients[i].is_zero());
        At[i] = libff::Fr<ppT>::zero();
    }
    //TODO check At are all 0 now ?
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
    //TODO try to change A_query
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
    //TODO try to change here because of Kt
    libff::G1_vector<ppT> K_query = batch_exp(libff::Fr<ppT>::size_in_bits(), g1_window, g1_table, Kt);

    
    /*At[cs.constraints[index].a.terms[0].index] = r1cs_to_qap_instance_map_with_evaluation_At(cs_copy, t, 
                                                    At[cs.constraints[index].a.terms[0].index], new_coef, 
                                                    coef_save, index);*/
    /*Kt[0] = r1cs_to_qap_instance_map_with_evaluation_Zt2(cs_copy, t, 
                                                    Kt[0], At_save, 
                                                    At[cs.constraints[index].a.terms[0].index], 0, rA, beta);*/
    /*cout << "Ic coef 0 before " << IC_coefficients[0] << endl;
    IC_coefficients[0] = At_save_index_0;
    cout << "Ic coef 0 after " << IC_coefficients[0] << endl;
    //Todo change here the KT 0 query
    G1<ppT> test = batch_exp_monomial(libff::Fr<ppT>::size_in_bits(), g1_window, g1_table, Kt[0]);
    cout << "Arguments value 3 : " << endl;
    cout << "   t " << t << endl;
    cout << "   K_query[0] 0 " << test << endl;
    cout << "   g1_window " << g1_window << endl;
    //cout << "   g1_table " << g1_table << endl;
    cout << "    Kt[0] " <<  Kt[0] << endl;*/
    //cout << "K_query r1cs_ppzksnark_generator_with_t_and_random " << K_query <<endl;
    //return ;
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
        //TODO change mul ic coef because of ic coef
        multiplied_IC_coefficients.emplace_back(rA * IC_coefficients[i]);
    }
    //TODO change because of ic mult coef
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

    pk.print_size();
    vk.print_size();

    return r1cs_ppzksnark_keypair<ppT>(std::move(pk), std::move(vk));
}

template <typename ppT>
std::tuple<r1cs_ppzksnark_keypair<ppT>,std::tuple<libff::Fr<ppT>, random_container_key<ppT>>> r1cs_ppzksnark_generator2(
    const r1cs_ppzksnark_constraint_system<ppT> &cs)
{
    libff::enter_block("Call to r1cs_ppzksnark_generator");

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

    //cout << "At r1cs_ppzksnark_generator2 " << At << endl;
    //cout << "end " << endl;
    /* append Zt to At,Bt,Ct with */
    At.emplace_back(qap_inst.Zt);
    Bt.emplace_back(qap_inst.Zt);
    Ct.emplace_back(qap_inst.Zt);
    
    Fr<ppT> At_save = At[0];

    //cout << "At save r1cs ppzksnark generator 2 " << At_save << endl;
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
                                                                        Kt[0],
                                                                        g1_window);

    pk.print_size();
    vk.print_size();
    std::tuple<libff::Fr<ppT>, random_container_key<ppT>> vec_return1(t, container_random);
    std::tuple<r1cs_ppzksnark_keypair<ppT>,std::tuple<libff::Fr<ppT>, random_container_key<ppT>>> vec_return(r1cs_ppzksnark_keypair<ppT>(std::move(pk), std::move(vk)), vec_return1);
    
    return vec_return;
}


template <typename ppT>
r1cs_ppzksnark_keypair<ppT> r1cs_ppzksnark_generator_with_t_and_random(const r1cs_ppzksnark_constraint_system<ppT> &cs, const libff::Fr<ppT> t, 
                    random_container_key<ppT> random_container, libff::Fr<ppT> coef_save, uint64_t coef_index, uint64_t degree)
{
    libff::enter_block("Call to r1cs_ppzksnark_generator_with_t");

    /* make the B_query "lighter" if possible */
    r1cs_ppzksnark_constraint_system<ppT> cs_copy(cs);
    cs_copy.swap_AB_if_beneficial();

    /* draw random element at which the QAP is evaluated */
    //const  libff::Fr<ppT> t = libff::Fr<ppT>::random_element();

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
    /*cout << "Kt r1cs_ppzksnark_generator_with_t_and_random " << Kt <<endl;*/
    /* zero out prefix of At and stick it into IC coefficients */
    libff::Fr_vector<ppT> IC_coefficients;
    IC_coefficients.reserve(qap_inst.num_inputs() + 1);
    for (size_t i = 0; i < qap_inst.num_inputs() + 1; ++i)
    {
        IC_coefficients.emplace_back(At[i]);
        assert(!IC_coefficients[i].is_zero());
        At[i] = libff::Fr<ppT>::zero();
    }
    /*cout << "IC_coeff r1cs_ppzksnark_generator_with_t_and_random " << IC_coefficients <<endl;
    cout << "IC_coefficients before r1cs_ppzksnark_generator_with_t_and_random " << IC_coefficients[0] <<endl;*/
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

    pk.print_size();
    vk.print_size();
    //std::tuple<libff::Fr<ppT>, std::shared_ptr<libfqfft::evaluation_domain<FieldT> >> vec_return1(t, qap_inst.domain);
    return r1cs_ppzksnark_keypair<ppT>(std::move(pk), std::move(vk));
}

template <typename ppT>
void compare_keypair(r1cs_ppzksnark_keypair<ppT> ref_keypair, r1cs_ppzksnark_keypair<ppT> new_keypair){
    libff::enter_block("Call to compare_keypair");

    libff::enter_block("Compare verification key");

    cout << "AlphaA_g2.....................";
    if(ref_keypair.vk.alphaA_g2 == new_keypair.vk.alphaA_g2){
        cout << "OK" << endl;
    } else {
        cout << "NOT OK" << endl;
    }

    cout << "alphaB_g1.....................";
    if(ref_keypair.vk.alphaB_g1 == new_keypair.vk.alphaB_g1){
        cout << "OK" << endl;
    } else {
        cout << "NOT OK" << endl;
    }

    cout << "alphaC_g2.....................";
    if(ref_keypair.vk.alphaC_g2 == new_keypair.vk.alphaC_g2){
        cout << "OK" << endl;
    } else {
        cout << "NOT OK" << endl;
    }

    cout << "gamma_g2......................";
    if(ref_keypair.vk.gamma_g2 == new_keypair.vk.gamma_g2){
        cout << "OK" << endl;
    } else {
        cout << "NOT OK" << endl;
    }

    cout << "gamma_beta_g1.................";
    if(ref_keypair.vk.gamma_beta_g1 == new_keypair.vk.gamma_beta_g1){
        cout << "OK" << endl;
    } else {
        cout << "NOT OK" << endl;
    }

    cout << "gamma_beta_g2.................";
    if(ref_keypair.vk.gamma_beta_g2 == new_keypair.vk.gamma_beta_g2){
        cout << "OK" << endl;
    } else {
        cout << "NOT OK" << endl;
    }

    cout << "rC_Z_g2.......................";
    if(ref_keypair.vk.rC_Z_g2 == new_keypair.vk.rC_Z_g2){
        cout << "OK" << endl;
    } else {
        cout << "NOT OK" << endl;
    }

    
    string text = "OK";
    if(ref_keypair.vk.encoded_IC_query.first == new_keypair.vk.encoded_IC_query.first){
        for(int i = 0; i < ref_keypair.vk.encoded_IC_query.rest.values.size() && i < new_keypair.vk.encoded_IC_query.rest.values.size(); i++){
            if(ref_keypair.vk.encoded_IC_query.rest.values[i] != new_keypair.vk.encoded_IC_query.rest.values[i]) {
                //cout << text << endl;
                cout << "index " << i  << " not ok for K_query "<< endl;
                text = "NOT OK";
                break;
            }
        }
        cout << "encoded_IC_query..............";
        cout << text << endl;
    } else {
        cout << "encoded_IC_query..............";
        cout << "NOT OK" << endl;
    }

    libff::leave_block("Compare verification key");

    libff::enter_block("Compare proving key");

    cout << "A_query.......................";
    
    text = "OK";
    for(int i = 0; i < ref_keypair.pk.A_query.values.size() && i < new_keypair.pk.A_query.values.size(); i++){
        if(ref_keypair.pk.A_query.values[i] != new_keypair.pk.A_query.values[i]) {
            text = "NOT OK";
            break;
        }
    }
    cout << text << endl;   
    
    cout << "B_query.......................";
    
    text = "OK";
    for(int i = 0; i < ref_keypair.pk.B_query.values.size() && i < new_keypair.pk.B_query.values.size(); i++){
        if(ref_keypair.pk.B_query.values[i] != new_keypair.pk.B_query.values[i]) {
            text = "NOT OK";
            break;
        }
    }
    cout << text << endl; 

    cout << "C_query.......................";
    
    text = "OK";
    for(int i = 0; i < ref_keypair.pk.C_query.values.size() && i < new_keypair.pk.C_query.values.size(); i++){
        if(ref_keypair.pk.C_query.values[i] != new_keypair.pk.C_query.values[i]) {
            text = "NOT OK";
            break;
        }
    }
    cout << text << endl; 

    cout << "H_query.......................";
    
    text = "OK";
    for(int i = 0; i < ref_keypair.pk.H_query.size() && i < new_keypair.pk.H_query.size(); i++){
        if(ref_keypair.pk.H_query[i] != new_keypair.pk.H_query[i]) {
            text = "NOT OK";
            break;
        }
    }
    cout << text << endl; 

    
    
    text = "OK";
    for(int i = 0; i < ref_keypair.pk.K_query.size() && i < new_keypair.pk.K_query.size(); i++){
        if(ref_keypair.pk.K_query[i] != new_keypair.pk.K_query[i]) {
            cout << "index " << i  << " not ok for K_query "<< endl;
            text = "NOT OK";
            break;
        }
    }
    cout << "K_query.......................";
    cout << text << endl; 

    libff::leave_block("Compare proving key");

    libff::leave_block("Call to compare_keypair");
}


template <typename ppT>
r1cs_ppzksnark_keypair<ppT> update_proving_key_compilation(const r1cs_ppzksnark_constraint_system<ppT> &cs,libff::Fr<ppT> coef_save, 
                    libff::Fr<ppT> new_coef, uint64_t coef_index, uint64_t degree, 
                    libff::Fr<ppT> t, random_container_key<ppT> random_container, r1cs_ppzksnark_keypair<ppT> ref_keypair)
{   
    Fr<ppT> At_save = random_container.At_save;
    random_container.At_save = r1cs_to_qap_instance_map_with_evaluation_At(cs, t, 
                        random_container.At_save, new_coef, 
                        coef_save, coef_index);
    Fr<ppT> At_save_index_0 = random_container.At_save;
    random_container.Kt_save = r1cs_to_qap_instance_map_with_evaluation_Zt2(cs, t, 
                        random_container.Kt_save, At_save, 
                        random_container.At_save, 0, random_container.rA, random_container.beta);
    libff::window_table<libff::G1<ppT> > g1_table = get_window_table(libff::Fr<ppT>::size_in_bits(), random_container.g1_window, libff::G1<ppT>::one());
    ref_keypair.pk.K_query[0] = batch_exp_monomial(libff::Fr<ppT>::size_in_bits(), random_container.g1_window, g1_table, random_container.Kt_save);
    libff::G1<ppT> encoded_IC_base = (random_container.rA * At_save_index_0) * libff::G1<ppT>::one();
    ref_keypair.vk.encoded_IC_query.first = encoded_IC_base;
    return r1cs_ppzksnark_keypair<ppT>(std::move(ref_keypair.pk), std::move(ref_keypair.vk));
}

} // libsnark
#endif // R1CS_PPZKSNARK_TCC_

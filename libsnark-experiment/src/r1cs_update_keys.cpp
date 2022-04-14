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

#ifdef MULTICORE
#include <omp.h>
#endif

#include <libsnark/knowledge_commitment/kc_multiexp.hpp>
#include <libsnark/reductions/r1cs_to_qap/r1cs_to_qap.hpp>

namespace libsnark {

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
r1cs_ppzksnark_keypair<ppT> r1cs_ppzksnark_generator55(const r1cs_ppzksnark_constraint_system<ppT> &cs)
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

    /* append Zt to At,Bt,Ct with */
    At.emplace_back(qap_inst.Zt);
    Bt.emplace_back(qap_inst.Zt);
    Ct.emplace_back(qap_inst.Zt);

    const  libff::Fr<ppT> alphaA = libff::Fr<ppT>::random_element(),
        alphaB = libff::Fr<ppT>::random_element(),
        alphaC = libff::Fr<ppT>::random_element(),
        rA = libff::Fr<ppT>::random_element(),
        rB = libff::Fr<ppT>::random_element(),
        beta = libff::Fr<ppT>::random_element(),
        gamma = libff::Fr<ppT>::random_element();
    const libff::Fr<ppT>      rC = rA * rB;

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

    /* zero out prefix of At and stick it into IC coefficients */
    libff::Fr_vector<ppT> IC_coefficients;
    IC_coefficients.reserve(qap_inst.num_inputs() + 1);
    for (size_t i = 0; i < qap_inst.num_inputs() + 1; ++i)
    {
        //TODO change IC coef
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

} // libsnark
#endif // R1CS_PPZKSNARK_TCC_

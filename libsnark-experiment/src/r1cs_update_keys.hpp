/**
 * @file r1cs_update_keys.hpp
 * @author Martinez Anthony
 * @brief Contained the defintion of the random_container_key class 
 * and some function to check the update of the key
 * @version 0.1
 * @date 2022-07-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef R1CS_UPDATE_KEYS_HPP
#define R1CS_UPDATE_KEYS_HPP
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <time.h>

#include "libff/algebra/fields/field_utils.hpp"
#include "libsnark/zk_proof_systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark.hpp"
#include "libsnark/common/default_types/r1cs_ppzksnark_pp.hpp"
#include "libsnark/gadgetlib1/pb_variable.hpp"

#include "libsnark/gadgetlib2/variable.hpp"
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
using namespace libsnark;
using namespace std;

/**
 * @brief This container is used to store needed informations 
 * for the update
 * 
 * @tparam ppT 
 */
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
    libff::Fr<ppT> At_save;
    libff::Fr<ppT> Bt_save;
    libff::Fr<ppT> Kt_save;
    random_container_key(libff::Fr<ppT> alphaA,
            libff::Fr<ppT> alphaB,
            libff::Fr<ppT> alphaC,
            libff::Fr<ppT> rA,
            libff::Fr<ppT> rB,
            libff::Fr<ppT> beta,
            libff::Fr<ppT> gamma,
            libff::Fr<ppT> At_save,
            libff::Fr<ppT> Bt_save,
            libff::Fr<ppT> Kt_save) :
            alphaA(alphaA),
            alphaB(alphaB),
            alphaC(alphaC),
            rA(rA),
            rB(rB),
            beta(beta),
            gamma(gamma),
            At_save(At_save),
            Bt_save(Bt_save),
            Kt_save(Kt_save){};
    random_container_key(){};
};

/**
 * @brief Container used to return element 
 * 
 * @tparam ppT 
 */
template< typename ppT >
class return_container_key_generator_for_update {
private: 
    libff::Fr<ppT> FFT_evaluation_point;
    random_container_key<ppT> random_container;
    r1cs_ppzksnark_keypair<ppT> key_pair;
public:
    return_container_key_generator_for_update(libff::Fr<ppT> FFT_evaluation_point,
            random_container_key<ppT> random_container,
            r1cs_ppzksnark_keypair<ppT> key_pair):
            FFT_evaluation_point(FFT_evaluation_point),
            random_container(random_container),
            key_pair(key_pair){};
    libff::Fr<ppT> get_FFT_evaluation_point(){
        return FFT_evaluation_point;
    }
    random_container_key<ppT> get_random_container(){
        return random_container;
    }
    r1cs_ppzksnark_keypair<ppT> get_key_pair(){
        return key_pair;
    }
};

/**
 * @brief Function used to generate a key that we can
 * compare to a key updated. 
 * 
 * @tparam ppT 
 * @param cs 
 * @param FFT_evaluation_point 
 * @param random_container 
 * @return r1cs_ppzksnark_keypair<ppT> 
 */
template <typename ppT>
r1cs_ppzksnark_keypair<ppT> r1cs_ppzksnark_generator_with_FFT_evaluation_point_and_random_values(const r1cs_ppzksnark_constraint_system<ppT> &cs, 
        const libff::Fr<ppT> FFT_evaluation_point, random_container_key<ppT> random_container);

/**
 * @brief Function used to compare the content of 2 keys
 * we have used it to see which elements of the key were 
 * updated
 * 
 * @tparam ppT 
 * @param ref_keypair 
 * @param new_keypair 
 */
template < typename ppT >
void compare_keypair(r1cs_ppzksnark_keypair<ppT> ref_keypair, r1cs_ppzksnark_keypair<ppT> new_keypair);

#include "r1cs_update_keys.tcc"

#endif 
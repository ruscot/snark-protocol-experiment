/** @file
 *****************************************************************************

 Implementation of interfaces for the "step radix-2" evaluation domain.

 See step_radix2_domain.hpp .

 *****************************************************************************
 * @author     This file is part of libfqfft, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef STEP_RADIX2_DOMAIN_TCC_

#include <libfqfft/evaluation_domain/domains/basic_radix2_domain_aux.hpp>

namespace libfqfft {

template<typename FieldT>
step_radix2_domain<FieldT>::step_radix2_domain(const size_t m) : evaluation_domain<FieldT>(m)
{
    if (m <= 1) throw InvalidSizeException("step_radix2(): expected m > 1");

    big_m = 1ul<<(libff::log2(m)-1);
    small_m = m - big_m;

    if (small_m != 1ul<<libff::log2(small_m))
        throw DomainSizeException("step_radix2(): expected small_m == 1ul<<log2(small_m)");

    try { omega = libff::get_root_of_unity<FieldT>(1ul<<libff::log2(m)); }
    catch (const std::invalid_argument& e) { throw DomainSizeException(e.what()); }
    
    big_omega = omega.squared();
    small_omega = libff::get_root_of_unity<FieldT>(small_m);
}

template<typename FieldT>
void step_radix2_domain<FieldT>::FFT(std::vector<FieldT> &a)
{
    if (a.size() != this->m) throw DomainSizeException("step_radix2: expected a.size() == this->m");

    std::vector<FieldT> c(big_m, FieldT::zero());
    std::vector<FieldT> d(big_m, FieldT::zero());

    FieldT omega_i = FieldT::one();
    for (size_t i = 0; i < big_m; ++i)
    {
        c[i] = (i < small_m ? a[i] + a[i+big_m] : a[i]);
        d[i] = omega_i * (i < small_m ? a[i] - a[i+big_m] : a[i]);
        omega_i *= omega;
    }

    std::vector<FieldT> e(small_m, FieldT::zero());
    const size_t compr = 1ul<<(libff::log2(big_m) - libff::log2(small_m));
    for (size_t i = 0; i < small_m; ++i)
    {
        for (size_t j = 0; j < compr; ++j)
        {
            e[i] += d[i + j * small_m];
        }
    }

    _basic_radix2_FFT(c, omega.squared());
    _basic_radix2_FFT(e, libff::get_root_of_unity<FieldT>(small_m));

    for (size_t i = 0; i < big_m; ++i)
    {
        a[i] = c[i];
    }

    for (size_t i = 0; i < small_m; ++i)
    {
        a[i+big_m] = e[i];
    }
}

template<typename FieldT>
void step_radix2_domain<FieldT>::iFFT(std::vector<FieldT> &a)
{
    if (a.size() != this->m) throw DomainSizeException("step_radix2: expected a.size() == this->m");

    std::vector<FieldT> U0(a.begin(), a.begin() + big_m);
    std::vector<FieldT> U1(a.begin() + big_m, a.end());

    _basic_radix2_FFT(U0, omega.squared().inverse());
    _basic_radix2_FFT(U1, libff::get_root_of_unity<FieldT>(small_m).inverse());

    const FieldT U0_size_inv = FieldT(big_m).inverse();
    for (size_t i = 0; i < big_m; ++i)
    {
        U0[i] *= U0_size_inv;
    }

    const FieldT U1_size_inv = FieldT(small_m).inverse();
    for (size_t i = 0; i < small_m; ++i)
    {
        U1[i] *= U1_size_inv;
    }

    std::vector<FieldT> tmp = U0;
    FieldT omega_i = FieldT::one();
    for (size_t i = 0; i < big_m; ++i)
    {
        tmp[i] *= omega_i;
        omega_i *= omega;
    }

    // save A_suffix
    for (size_t i = small_m; i < big_m; ++i)
    {
        a[i] = U0[i];
    }

    const size_t compr = 1ul<<(libff::log2(big_m) - libff::log2(small_m));
    for (size_t i = 0; i < small_m; ++i)
    {
        for (size_t j = 1; j < compr; ++j)
        {
            U1[i] -= tmp[i + j * small_m];
        }
    }

    const FieldT omega_inv = omega.inverse();
    FieldT omega_inv_i = FieldT::one();
    for (size_t i = 0; i < small_m; ++i)
    {
        U1[i] *= omega_inv_i;
        omega_inv_i *= omega_inv;
    }

    // compute A_prefix
    const FieldT over_two = FieldT(2).inverse();
    for (size_t i = 0; i < small_m; ++i)
    {
        a[i] = (U0[i]+U1[i]) * over_two;
    }

    // compute B2
    for (size_t i = 0; i < small_m; ++i)
    {
        a[big_m + i] = (U0[i]-U1[i]) * over_two;
    }
}

template<typename FieldT>
void step_radix2_domain<FieldT>::cosetFFT(std::vector<FieldT> &a, const FieldT &g)
{
    _multiply_by_coset(a, g);
    FFT(a);
}

template<typename FieldT>
void step_radix2_domain<FieldT>::icosetFFT(std::vector<FieldT> &a, const FieldT &g)
{
    iFFT(a);
    _multiply_by_coset(a, g.inverse());
}

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
FieldT step_radix2_domain<FieldT>::evaluate_one_lagrange_polynomials(const FieldT &t, uint64_t index)
{
    std::cout << "Evaluate one lagrange polynomials" << std::endl;
    FieldT result = FieldT::zero();
    bool omega_i_equal_t = false;
    if(index < big_m){
        const FieldT L0 = (t^small_m)-(omega^small_m);

        const FieldT omega_to_small_m = omega^small_m;
        
        FieldT big_omega_to_small_m = big_omega ^ small_m;

        FieldT elt = FieldT::one();

        //Function basic_radix for inner_big
        FieldT inner_big;
        if(big_m == 1){
            inner_big = FieldT::one();
        } else {
            if (big_m != (1u << libff::log2(big_m))) throw DomainSizeException("expected big_m == (1u << log2(big_m))");
            FieldT omega = libff::get_root_of_unity<FieldT>(big_m);
            inner_big = FieldT::zero();
            if ((t^big_m) == (FieldT::one()))
            {
                FieldT omega_i = FieldT::one();
                for(uint64_t FE_index = index; FE_index; FE_index >>= 1)
                {
                    if (FE_index & 1)
                        omega_i *= omega;
                    omega *= omega;
                }
                if(omega_i == t){
                    inner_big = FieldT::one();
                    omega_i_equal_t = true;
                }

            } 
            if(!omega_i_equal_t) {
                const FieldT Z = (t^big_m)-FieldT::one();

                FieldT l = Z * FieldT(big_m).inverse();
                FieldT r = FieldT::one();
                omega = libff::get_root_of_unity<FieldT>(big_m);

                for(uint64_t FE_index = index; FE_index; FE_index >>= 1)
                {
                    if (FE_index & 1){
                        l *= omega;
                        r *= omega;
                    }
                    omega *= omega;
                }
                inner_big = l * (t - r).inverse();
            }
        }
        

        //Fast exponentiation 
        for(uint64_t FE_index = index; FE_index; FE_index >>= 1)
        {
            if (FE_index & 1)
                elt *= big_omega_to_small_m;
            big_omega_to_small_m *= big_omega_to_small_m;
        }
        result= inner_big * L0 * (elt - omega_to_small_m).inverse();

    }
    
    size_t i = index - big_m;
    
    //Function basic_radix for inner_small
    FieldT t_omaga_inv = t * omega.inverse();
    omega_i_equal_t = false;
    FieldT inner_small;
    if(small_m == 1){
        inner_small = FieldT::one();
    } else {
        if (small_m != (1u << libff::log2(small_m))) throw DomainSizeException("expected small_m == (1u << log2(small_m))");
        FieldT omega = libff::get_root_of_unity<FieldT>(small_m);
        inner_small = FieldT::zero();
        bool omega_i_equal_t = false;
        if ((t_omaga_inv^small_m) == (FieldT::one()))
        {
            FieldT omega_i = FieldT::one();

            for(uint64_t FE_index = index - big_m; FE_index; FE_index >>= 1)
            {
                if (FE_index & 1)
                    omega_i *= omega;
                omega *= omega;
            }
            if(omega_i == t){
                inner_small = FieldT::one();
                omega_i_equal_t = true;
            }
        }
        if(!omega_i_equal_t) {
            const FieldT Z = (t_omaga_inv^small_m)-FieldT::one();
            FieldT l = Z * FieldT(small_m).inverse();
            FieldT r = FieldT::one();
 
            omega = libff::get_root_of_unity<FieldT>(small_m);

            for(uint64_t FE_index = index - big_m; FE_index; FE_index >>= 1)
            {
                if (FE_index & 1){
                    l *= omega;
                    r *= omega;
                }
                omega *= omega;
            }
            inner_small = l * (t_omaga_inv - r).inverse();
        }
    }


    if(i < small_m){
        const FieldT L1 = ((t^big_m)-FieldT::one()) * ((omega^big_m) - FieldT::one()).inverse();
        result = L1 * inner_small;
    }

    return result;
}

template<typename FieldT>
std::vector<FieldT> step_radix2_domain<FieldT>::evaluate_all_lagrange_polynomials(const FieldT &t)
{   
    std::cout << "Evaluate all lagrange polynomials step_radix2_domain" << std::endl;
    std::vector<FieldT> inner_big = _basic_radix2_evaluate_all_lagrange_polynomials(big_m, t);
    std::vector<FieldT> inner_small = _basic_radix2_evaluate_all_lagrange_polynomials(small_m, t * omega.inverse());

    std::vector<FieldT> result(this->m, FieldT::zero());

    const FieldT L0 = (t^small_m)-(omega^small_m);
    const FieldT omega_to_small_m = omega^small_m;
    const FieldT big_omega_to_small_m = big_omega ^ small_m;
    FieldT elt = FieldT::one();
    for (size_t i = 0; i < big_m; ++i)
    {
        result[i] = inner_big[i] * L0 * (elt - omega_to_small_m).inverse();
        elt *= big_omega_to_small_m;
    }

    const FieldT L1 = ((t^big_m)-FieldT::one()) * ((omega^big_m) - FieldT::one()).inverse();

    for (size_t i = 0; i < small_m; ++i)
    {
        result[big_m + i] = L1 * inner_small[i];
    }

    return result;
}

template<typename FieldT>
FieldT step_radix2_domain<FieldT>::get_domain_element(const size_t idx)
{
    if (idx < big_m)
    {
        return big_omega^idx;
    }
    else
    {
        return omega * (small_omega^(idx-big_m));
    }
}

template<typename FieldT>
FieldT step_radix2_domain<FieldT>::compute_vanishing_polynomial(const FieldT &t)
{
    return ((t^big_m) - FieldT::one()) * ((t^small_m) - (omega^small_m));
}

template<typename FieldT>
void step_radix2_domain<FieldT>::add_poly_Z(const FieldT &coeff, std::vector<FieldT> &H)
{
    if (H.size() != this->m+1) throw DomainSizeException("step_radix2: expected H.size() == this->m+1");

    const FieldT omega_to_small_m = omega^small_m;

    H[this->m] += coeff;
    H[big_m] -= coeff * omega_to_small_m;
    H[small_m] -= coeff;
    H[0] += coeff * omega_to_small_m;
}

template<typename FieldT>
void step_radix2_domain<FieldT>::divide_by_Z_on_coset(std::vector<FieldT> &P)
{
    // (c^{2^k}-1) * (c^{2^r} * w^{2^{r+1}*i) - w^{2^r})
    const FieldT coset = FieldT::multiplicative_generator;

    const FieldT Z0 = (coset^big_m) - FieldT::one();
    const FieldT coset_to_small_m_times_Z0 = (coset^small_m) * Z0;
    const FieldT omega_to_small_m_times_Z0 = (omega^small_m) * Z0;
    const FieldT omega_to_2small_m = omega^(2*small_m);
    FieldT elt = FieldT::one();

    for (size_t i = 0; i < big_m; ++i)
    {
        P[i] *= (coset_to_small_m_times_Z0 * elt - omega_to_small_m_times_Z0).inverse();
        elt *= omega_to_2small_m;
    }

    // (c^{2^k}*w^{2^k}-1) * (c^{2^k} * w^{2^r} - w^{2^r})

    const FieldT Z1 = ((((coset*omega)^big_m) - FieldT::one()) * (((coset * omega)^small_m) - (omega^small_m)));
    const FieldT Z1_inverse = Z1.inverse();

    for (size_t i = 0; i < small_m; ++i)
    {
        P[big_m + i] *= Z1_inverse;
    }

}

} // libfqfft

#endif // STEP_RADIX2_DOMAIN_TCC_

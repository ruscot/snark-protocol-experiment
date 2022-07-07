#include <tuple>
#include "r1cs_update_keys.cpp"
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

template<typename FieldT, typename ppT>
class R1CS_Polynomial_factory{
public:

    R1CS_Polynomial_factory(uint64_t degree){
        this->polynomial_degree = degree;
    }

    uint64_t get_polynomial_degree(){
        return this->polynomial_degree;
    }
    
    vector<libff::Fr<ppT>> get_random_polynomial(){
        return this->poly;
    }

    libff::Fr<ppT> get_polynomial_coefficients(uint64_t coef){
        return this->poly[coef];
    }

    pb_variable<FieldT> get_x_variable(){
        return this->x;
    }

    pb_variable<FieldT> get_y_variable(){
        return this->out;
    }

    void update_polynomial_coefficient(libff::Fr<ppT> new_coef, uint64_t index_of_coefficient){
        this->poly[index_of_coefficient] = new_coef;
    }

    /**
     * @brief Create a polynomials of degree "degree" with random coefficient in the field "ppT".
     * 
     * @tparam ppT 
     * @param degree 
     * @return vector<libff::Fr<ppT>> 
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
    void create_constraint_horner_method(protoboard<FieldT> *pb)
    {
        libff::enter_block("Create constraint horner method");
        this->out.allocate(*pb, "out");
        this->x.allocate(*pb, "x");

        if(this->polynomial_degree == 0){
            printf("Not a polynomial\n");
        } else {
            pb_variable<FieldT> last_var_1;
            last_var_1.allocate(*pb, "0");
            (*pb).add_r1cs_constraint(r1cs_constraint<FieldT>(this->x, this->poly[this->polynomial_degree], last_var_1));
            string last_var_name = "1";
            for(uint64_t i = this->polynomial_degree; i > 1; i-=1){
                pb_variable<FieldT> last_var_2;
                last_var_name = std::to_string(stoi(last_var_name) + 1);
                last_var_2.allocate(*pb, last_var_name);
                (*pb).add_r1cs_constraint(r1cs_constraint<FieldT>(this->poly[i-1] + last_var_1, this->x, last_var_2));
                last_var_1 = last_var_2;
            }
            (*pb).add_r1cs_constraint(r1cs_constraint<FieldT>(this->poly[0] + last_var_1, 1, this->out));
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
        libff::Fr<ppT> x_value = this->protoboard_for_poly.auxiliary_input()[0];
        libff::Fr<ppT> res = this->poly[this->polynomial_degree];
        for(uint64_t i = this->polynomial_degree; i > 0; i-=1){
            res = res*x_value + this->poly[i-1];
        }
        return res;
    }

    void update_constraint_horner_method(uint64_t coef_index)
    {
        libff::enter_block("Update constraint");
        if(coef_index == this->polynomial_degree){
            r1cs_constraint<FieldT> constraint = ((this->protoboard_for_poly)).get_specific_constraint_in_r1cs(0);
            constraint.b.terms[0].coeff = this->poly[coef_index];
            ((this->protoboard_for_poly)).protoboard_update_r1cs_constraint(constraint, 0);
        } else if (coef_index == this->polynomial_degree - 1) {
            r1cs_constraint<FieldT> constraint = ((this->protoboard_for_poly)).get_specific_constraint_in_r1cs(1);
            constraint.a.terms[0].coeff = this->poly[coef_index];
            ((this->protoboard_for_poly)).protoboard_update_r1cs_constraint(constraint, 1);
        } else {
            uint64_t index = this->polynomial_degree - coef_index;
            r1cs_constraint<FieldT> constraint = ((this->protoboard_for_poly)).get_specific_constraint_in_r1cs(index);
            constraint.a.terms[0].coeff = this->poly[coef_index];
            ((this->protoboard_for_poly)).protoboard_update_r1cs_constraint(constraint, index);
        }
        libff::leave_block("Update constraint");
    }

    void set_protoboard(protoboard<FieldT> protoboard_for_poly){
        this->protoboard_for_poly = protoboard_for_poly;
    }

    protoboard<FieldT> get_protoboard(){
        return this->protoboard_for_poly;
    }

    void set_constraint_system(r1cs_constraint_system<FieldT> constraint_system){
        this->constraint_system = constraint_system;
    }

    FieldT r1cs_to_qap_instance_map_with_evaluation_Bt(const FieldT &t, FieldT Bt, 
                FieldT new_coef, FieldT coef_save, uint64_t index)
    {
        libff::enter_block("Call to r1cs_to_qap_instance_map_with_evaluation_Bt");

        const std::shared_ptr<libfqfft::evaluation_domain<FieldT> > domain = libfqfft::get_evaluation_domain<FieldT>(constraint_system.num_constraints() + constraint_system.num_inputs() + 1);
        const FieldT u = domain->evaluate_one_lagrange_polynomials(t, index);
        Bt -= u * coef_save;
        Bt += u * new_coef; 
        libff::leave_block("Call to r1cs_to_qap_instance_map_with_evaluation_Bt");
        return Bt;
    }

    FieldT r1cs_to_qap_instance_map_with_evaluation_At(const FieldT &t, FieldT At, 
                FieldT new_coef, FieldT coef_save, uint64_t index)
    {
        libff::enter_block("Call to r1cs_to_qap_instance_map_with_evaluation_At");
        const std::shared_ptr<libfqfft::evaluation_domain<FieldT> > domain = libfqfft::get_evaluation_domain<FieldT>(constraint_system.num_constraints() + constraint_system.num_inputs() + 1);
        const FieldT u = domain->evaluate_one_lagrange_polynomials(t, index);
        At -= u * coef_save;
        At += u * new_coef; 
        libff::leave_block("Call to r1cs_to_qap_instance_map_with_evaluation_At");
        return At;

    }

    r1cs_ppzksnark_keypair<ppT> update_proving_key_compilation(libff::Fr<ppT> coef_save,  uint64_t coef_to_update,
                        uint64_t coef_index, 
                        libff::Fr<ppT> t, r1cs_ppzksnark_keypair<ppT> ref_keypair){
        if(0 == coef_index){
            Fr<ppT> Bt_save = random_container.get_Bt_save();
            random_container.set_Bt_save(r1cs_to_qap_instance_map_with_evaluation_Bt(t, 
                                random_container.get_Bt_save(), this->poly[coef_to_update], 
                                coef_save, coef_index));
            random_container.set_Kt_save(r1cs_to_qap_instance_map_with_evaluation_Zt2(
                                random_container.get_Kt_save(), Bt_save, 
                                random_container.get_Bt_save(), random_container.get_rB(), random_container.get_beta()));

            libff::window_table<libff::G1<ppT> > g1_table = random_container.get_g1_table();
            
            ref_keypair.pk.K_query[0] = batch_exp_monomial(libff::Fr<ppT>::size_in_bits(), random_container.get_g1_window(), 
                            g1_table, random_container.get_Kt_save());
            
            libff::window_table<libff::G2<ppT> > g2_table = random_container.get_g2_table();

            ref_keypair.pk.B_query.values[0] = knowledge_commitment<libff::G2<ppT>, libff::G1<ppT>>(
                                        windowed_exp(libff::Fr<ppT>::size_in_bits(), random_container.get_g2_window(), 
                                                        g2_table, random_container.get_rB() * random_container.get_Bt_save()),
                                        windowed_exp(libff::Fr<ppT>::size_in_bits(), random_container.get_g1_window(), 
                                                        g1_table, random_container.get_rB() * random_container.get_alphaB() * random_container.get_Bt_save()));
            return r1cs_ppzksnark_keypair<ppT>(std::move(ref_keypair.pk), std::move(ref_keypair.vk));
        } else {
            Fr<ppT> At_save = random_container.get_At_save();
            random_container.set_At_save(r1cs_to_qap_instance_map_with_evaluation_At(t, 
                                random_container.get_At_save(), this->poly[coef_to_update], 
                                coef_save, coef_index));
            Fr<ppT> At_save_index_0 = random_container.get_At_save();
            random_container.set_Kt_save(r1cs_to_qap_instance_map_with_evaluation_Zt2(random_container.get_Kt_save(), At_save, 
                                random_container.get_At_save(), random_container.get_rA(), random_container.get_beta()));

            libff::window_table<libff::G1<ppT> > g1_table = random_container.get_g1_table();

            ref_keypair.pk.K_query[0] = batch_exp_monomial(libff::Fr<ppT>::size_in_bits(), random_container.get_g1_window(), g1_table, random_container.get_Kt_save());
            
            libff::G1<ppT> encoded_IC_base = (random_container.get_rA() * At_save_index_0) * libff::G1<ppT>::one();
            
            ref_keypair.vk.encoded_IC_query.first = encoded_IC_base;
            return r1cs_ppzksnark_keypair<ppT>(std::move(ref_keypair.pk), std::move(ref_keypair.vk));
        }
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

private:
    vector<libff::Fr<ppT>> poly;
    uint64_t polynomial_degree;
    r1cs_constraint_system<FieldT> constraint_system;

    pb_variable<FieldT> out;
    pb_variable<FieldT> x;
    protoboard<FieldT> protoboard_for_poly;
    random_container_key<ppT> random_container;
};
#include <tuple>

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

    std::tuple<pb_variable<FieldT>,pb_variable<FieldT>> get_out_and_in_variable(){
        std::tuple<pb_variable<FieldT>,pb_variable<FieldT>> return_value;
        std::get<0>(return_value) = x;
        std::get<1>(return_value) = out;
        return return_value;
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
     * @brief An annex function to create our R1CS constraint with the horner method
     * 
     * @tparam FieldT Field for the constraint
     * @tparam ppT Field of the coeffictient of the polynomial "poly"
     * @param x_exponent Current coefficient of our polynomial
     * @param pb The protoboard who will stock our R1CS constraint
     * @param x The variable x of our polynomial
     * @param last_var The last variable used for our R1CS
     * @param last_var_name The last variable name for our R1CS
     * @return pb_variable<FieldT> 
     */
    pb_variable<FieldT> create_constraint_for_x_coefficient_horner(libff::Fr<ppT> x_coefficient, protoboard<FieldT> *pb, 
         pb_variable<FieldT> last_var, string *last_var_name)
    {
        int name_value = stoi(*last_var_name);
        pb_variable<FieldT> last_var_1;
        *last_var_name = std::to_string(name_value + 1);
        last_var_1.allocate(*pb, *last_var_name);
        (*pb).add_r1cs_constraint(r1cs_constraint<FieldT>(x_coefficient + last_var, this->x, last_var_1));
        return last_var_1;
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
                last_var_1 = create_constraint_for_x_coefficient_horner(this->poly[i-1], pb, last_var_1, &last_var_name);
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
        Chrono c_setup; 
        double get_constraint=0., change_coef=0., update_constraint=0.;
        libff::enter_block("Update constraint");

        if(coef_index == this->polynomial_degree){
            c_setup.start();
            r1cs_constraint<FieldT> constraint = ((this->protoboard_for_poly)).get_specific_constraint_in_r1cs(0);
            get_constraint = c_setup.stop();
            c_setup.start();
            constraint.b.terms[0].coeff = this->poly[coef_index];
            change_coef = c_setup.stop();
            c_setup.start();
            ((this->protoboard_for_poly)).protoboard_update_r1cs_constraint(constraint, 0);
            update_constraint = c_setup.stop();
        } else if (coef_index == this->polynomial_degree - 1) {
            c_setup.start();
            r1cs_constraint<FieldT> constraint = ((this->protoboard_for_poly)).get_specific_constraint_in_r1cs(1);
            get_constraint = c_setup.stop();
            c_setup.start();
            constraint.a.terms[0].coeff = this->poly[coef_index];
            change_coef = c_setup.stop();
            c_setup.start();
            ((this->protoboard_for_poly)).protoboard_update_r1cs_constraint(constraint, 1);
            update_constraint = c_setup.stop();
        } else {
            c_setup.start();
            uint64_t index = this->polynomial_degree - coef_index;
            r1cs_constraint<FieldT> constraint = ((this->protoboard_for_poly)).get_specific_constraint_in_r1cs(index);
            get_constraint = c_setup.stop();
            c_setup.start();
            constraint.a.terms[0].coeff = this->poly[coef_index];
            change_coef = c_setup.stop();
            c_setup.start();
            ((this->protoboard_for_poly)).protoboard_update_r1cs_constraint(constraint, index);
            update_constraint = c_setup.stop();
        }
        printf("[TIMINGS for r1cs] | get_constraint : %f | change_coef : %f | update_constraint : %f \n=== end ===\n\n", 
                get_constraint, change_coef, update_constraint);
        libff::leave_block("Update constraint");
    }

    void set_protoboard(protoboard<FieldT> protoboard_for_poly){
        this->protoboard_for_poly = protoboard_for_poly;
    }

    protoboard<FieldT> get_protoboard(){
        return this->protoboard_for_poly;
    }

private:
    vector<libff::Fr<ppT>> poly;
    uint64_t polynomial_degree;

    pb_variable<FieldT> out;
    pb_variable<FieldT> x;
    protoboard<FieldT> protoboard_for_poly;
};
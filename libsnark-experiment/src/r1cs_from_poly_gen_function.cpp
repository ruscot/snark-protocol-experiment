#include <tuple>

template<typename FieldT, typename ppT>
class R1CS_Polynomial_factory{
public:

    R1CS_Polynomial_factory(uint64_t degree){
        this->polynomial_degree = degree;
    }

    vector<libff::Fr<ppT>> get_random_polynomial(){
        return this->poly;
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

    std::tuple<pb_variable<FieldT>,pb_variable<FieldT>> get_out_and_in_variable(){
        std::tuple<pb_variable<FieldT>,pb_variable<FieldT>> return_value;
        std::get<0>(return_value) = x;
        std::get<1>(return_value) = out;
        return return_value;
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
        libff::enter_block("Create constraint");
        this->out.allocate(*pb, "out");
        this->x.allocate(*pb, "x");
        
        if(this->polynomial_degree == 0){
            printf("Not a polynomial\n");
        } else {
            pb_variable<FieldT> last_var_1;
            last_var_1.allocate(*pb, "0");
            (*pb).add_r1cs_constraint(r1cs_constraint<FieldT>(this->x, this->poly[this->polynomial_degree], last_var_1));
            if(this->polynomial_degree == 1){
                (*pb).add_r1cs_constraint(r1cs_constraint<FieldT>(last_var_1 + this->poly[this->polynomial_degree-1], 1, out));
            } else {
                pb_variable<FieldT> last_var_2;
                last_var_2.allocate(*pb, "1");
                (*pb).add_r1cs_constraint(r1cs_constraint<FieldT>(last_var_1 + this->poly[this->polynomial_degree-1], this->x, last_var_2));
                string last_var_name = "2";
                for(uint64_t i = this->polynomial_degree-1; i > 0; i-=1){
                    if(i == 1){
                        (*pb).add_r1cs_constraint(r1cs_constraint<FieldT>(this->poly[i-1] + last_var_2, 1, this->out));
                    } else {
                        last_var_2 = create_constraint_for_x_coefficient_horner(this->poly[i-1], pb, last_var_2, &last_var_name);
                    }
                    
                }
            }
        }
        
        libff::leave_block("Create constraint");
        
    }


    /**
     * @brief Just a function to check our computation with the R1CS. Compute the polynomial on "x_value" with the horner method
     * 
     * @tparam ppT The field of the polynomial
     * @param x_value The value of x
     * @return libff::Fr<ppT> The result of our computation
     */
    libff::Fr<ppT> evaluation_polynomial_horner(libff::Fr<ppT> x_value) {
        libff::Fr<ppT> res = this->poly[this->polynomial_degree];
        for(uint64_t i = this->polynomial_degree; i > 0; i-=1){
            res = res*x_value + this->poly[i-1];
        }
        return res;
    }

    void update_constraint_horner_method(libff::Fr<ppT> coef, protoboard<FieldT> *pb, uint64_t coef_index, uint64_t degree)
    {
        libff::enter_block("Update constraint");
    
        if(coef_index == degree){
            r1cs_constraint<FieldT> constraint = (*pb).get_constraint_system().constraints[0];
            constraint.b.terms[0].coeff = coef;
            (*pb).protoboard_update_r1cs_constraint(constraint, 0, "");
        } else if (coef_index == degree - 1) {
            r1cs_constraint<FieldT> constraint = (*pb).get_constraint_system().constraints[1];
            constraint.a.terms[0].coeff = coef;
            (*pb).protoboard_update_r1cs_constraint(constraint, 1, "");
        } else {
            uint64_t index = degree - coef_index;
            r1cs_constraint<FieldT> constraint = (*pb).get_constraint_system().constraints[index];
            constraint.a.terms[0].coeff = coef;
            (*pb).protoboard_update_r1cs_constraint(constraint, index, "");
        }
        libff::leave_block("Update constraint");
    }

private:
    vector<libff::Fr<ppT>> poly;
    uint64_t polynomial_degree;

    pb_variable<FieldT> out;
    pb_variable<FieldT> x;
};
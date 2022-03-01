#include <tuple>
/**
 * @brief Create a polynomials of degree "degree" with random coefficient in the field "FieldT".
 * 
 * @tparam FieldT 
 * @param degree 
 * @return vector<libff::Fr<FieldT>> 
 */
template<typename FieldT>
vector<libff::Fr<FieldT>> create_polynomials(uint64_t degree){
    vector<libff::Fr<FieldT>> poly;
    for(size_t i = 0; i <= degree; ++i) {
        //poly.push_back(libff::Fr<FieldT>::random_element());
        poly.push_back(2);
    }
    return poly;
}

template<typename FieldT>
vector<libff::Fr<FieldT>> polynomials_add_M(uint64_t degree, vector<libff::Fr<FieldT>> polynomial, libff::Fr<FieldT> M){
    vector<libff::Fr<FieldT>> poly;
    libff::Fr<FieldT> val = 1;
    for(size_t i = 0; i <= degree; ++i) {
        poly.push_back(polynomial[i] + val);
        val *= M;
    }
    return poly;
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
template<typename FieldT, typename ppT>
pb_variable<FieldT> create_constraint_for_x_exponent_horner(libff::Fr<ppT> x_exponent, protoboard<FieldT> *pb, 
    pb_variable<FieldT> x, pb_variable<FieldT> last_var, string *last_var_name)
{
    int name_value = stoi(*last_var_name);
    pb_variable<FieldT> last_var_1;
    *last_var_name = std::to_string(name_value + 1);
    last_var_1.allocate(*pb, *last_var_name);
    (*pb).add_r1cs_constraint(r1cs_constraint<FieldT>(x, last_var, last_var_1));
    pb_variable<FieldT> last_var_2;
    *last_var_name = std::to_string(name_value + 2);
    last_var_2.allocate(*pb, *last_var_name);
    (*pb).add_r1cs_constraint(r1cs_constraint<FieldT>(x_exponent + last_var_1, 1, last_var_2));
    return last_var_2;
}

template<typename FieldT, typename ppT>
void create_constraint_for_x_exponent_horner_out(libff::Fr<ppT> x_exponent, protoboard<FieldT> *pb, 
    pb_variable<FieldT> x, pb_variable<FieldT> last_var, string *last_var_name, pb_variable<FieldT> out)
{
    int name_value = stoi(*last_var_name);
    pb_variable<FieldT> last_var_1;
    *last_var_name = std::to_string(name_value + 1);
    last_var_1.allocate(*pb, *last_var_name);
    (*pb).add_r1cs_constraint(r1cs_constraint<FieldT>(x, last_var, last_var_1));
    //pb_variable<FieldT> last_var_2;
    //*last_var_name = std::to_string(name_value + 2);
    //last_var_2.allocate(*pb, *last_var_name);
    (*pb).add_r1cs_constraint(r1cs_constraint<FieldT>(x_exponent + last_var_1, 1, out));
    //return last_var_2;
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
template<typename FieldT, typename ppT>
std::tuple<pb_variable<FieldT>,pb_variable<FieldT>> create_constraint_horner_method(vector<libff::Fr<ppT>> poly, protoboard<FieldT> *pb, uint64_t degree)
{
    libff::enter_block("Create constraint");
    std::tuple<pb_variable<FieldT>,pb_variable<FieldT>> return_value;
    pb_variable<FieldT> out;
    out.allocate(*pb, "out");
    pb_variable<FieldT> x;
    x.allocate(*pb, "x");
    
    if(degree == 0){
        printf("Not a polynomial\n");
    } else {
        pb_variable<FieldT> last_var_1;
        last_var_1.allocate(*pb, "0");
        (*pb).add_r1cs_constraint(r1cs_constraint<FieldT>(x, poly[degree], last_var_1));
        if(degree == 1){
            //pb_variable<FieldT> last_var_2;
            //last_var_2.allocate(*pb, "1");
            //out.allocate(*pb, "out");
            (*pb).add_r1cs_constraint(r1cs_constraint<FieldT>(last_var_1 + poly[degree-1], 1, out));
        } else {
            pb_variable<FieldT> last_var_2;
            last_var_2.allocate(*pb, "1");
            (*pb).add_r1cs_constraint(r1cs_constraint<FieldT>(last_var_1 + poly[degree-1], 1, last_var_2));
            string last_var_name = "2";
            for(uint64_t i = degree-1; i > 0; i-=1){
                if(i == 1){
                    //out.allocate(*pb, "out");
                    //printf("On est la *--------------------------------------\n");
                    create_constraint_for_x_exponent_horner_out<FieldT, ppT>(poly[i-1], pb, x, last_var_2, &last_var_name, out);
                } else {
                    last_var_2 = create_constraint_for_x_exponent_horner<FieldT, ppT>(poly[i-1], pb, x, last_var_2, &last_var_name);
                }
                
            }
        }
    }
    libff::leave_block("Create constraint");
    std::get<0>(return_value) = x;
    std::get<1>(return_value) = out;
    return return_value;
}
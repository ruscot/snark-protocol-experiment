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
        poly.push_back(libff::Fr<FieldT>::random_element());
        //poly.push_back(2);
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
            (*pb).add_r1cs_constraint(r1cs_constraint<FieldT>(last_var_1 + poly[degree-1], 1, out));
        } else {
            pb_variable<FieldT> last_var_2;
            last_var_2.allocate(*pb, "1");
            (*pb).add_r1cs_constraint(r1cs_constraint<FieldT>(last_var_1 + poly[degree-1], 1, last_var_2));
            string last_var_name = "2";
            for(uint64_t i = degree-1; i > 0; i-=1){
                if(i == 1){
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

template<typename FieldT, typename ppT>
void update_constraint_horner_method(libff::Fr<ppT> coef, protoboard<FieldT> *pb, uint64_t coef_index, uint64_t degree)
{
    libff::enter_block("Update constraint");
    /*r1cs_constraint<FieldT> constraint;
    for(uint64_t i = 0; i < degree ; i++){
        cout << "i " << i << endl;
        constraint = (*pb).get_constraint_system().constraints[i];
        cout << "r1cs constraint a " << endl;
        constraint.a.print();
        cout << constraint.a.terms[0].coeff << endl;
        cout << constraint.a.terms[0].index << endl;
        cout << "r1cs constraint b " << endl;
        constraint.b.print();
        cout << constraint.b.terms[0].coeff << endl;
    }*/
  
    if(coef_index == degree){
        r1cs_constraint<FieldT> constraint = (*pb).get_constraint_system().constraints[0];
        cout << "r1cs constraint b " << endl;
        cout << "coef  " << coef << endl;
        cout << constraint.b.terms[0].coeff << endl;
        constraint.b.terms[0].coeff = coef;
        cout << constraint.b.terms[0].coeff << endl;
        (*pb).protoboard_update_r1cs_constraint(constraint, 0, "");
    } else if (coef_index == degree - 1) {
        r1cs_constraint<FieldT> constraint = (*pb).get_constraint_system().constraints[1];
        cout << "r1cs constraint a " << endl;
        constraint.a.print();
        cout << constraint.a.terms[0].coeff << endl;
        cout << constraint.a.terms[0].index << endl;
        constraint.a.terms[0].coeff = coef;
        cout << "coef " << coef << endl;
        constraint.a.print();
        (*pb).protoboard_update_r1cs_constraint(constraint, 1, "");
    } else {
        uint64_t index = 2 + (degree - coef_index - 2) * 2 +1;
        cout << "index " << index << endl;
        r1cs_constraint<FieldT> constraint = (*pb).get_constraint_system().constraints[index];
        cout << "r1cs constraint a " << endl;
        constraint.a.print();
        constraint.a.terms[0].coeff = coef;
        cout << "coef " << coef << endl;
        constraint.a.print();
        (*pb).protoboard_update_r1cs_constraint(constraint, index, "");
    }
    
    libff::leave_block("Update constraint");
}

  /*
21812035055494624812787781580588190355381051726023411839889764010840555760197
18264955162797063752559944305439423604666780529402049673784088251426451961327
6280039041269993045744033927058262086663646205129951997185127168875474770666
21191741919942421796898621929722566307170111953122137131681507602972831011837
7786041477537073298857969679157220926233805807131900586076506215427676761593
973773522213953214195682434770678475784580037887673996447341121400076089101
31673561585488433599050791904540180595868436979789025252305780393337142503
7291511875036787203678836918395449182321513594297117053754470978267401673272
13886149822949901285820179172059501909624150799966725865910396498661827981419
19504457738222001575628129932762382111552964536682514414687117191202960254204


i 0
r1cs constraint a 
here1    x_2 (no annotation) * 1
1
2
r1cs constraint b 
here    1 * 19504457738222001575628129932762382111552964536682514414687117191202960254204
19504457738222001575628129932762382111552964536682514414687117191202960254204
i 1
r1cs constraint a 
here    1 * 13886149822949901285820179172059501909624150799966725865910396498661827981419
here1    x_3 (no annotation) * 1
13886149822949901285820179172059501909624150799966725865910396498661827981419
0
r1cs constraint b 
here    1 * 1
1
i 2
r1cs constraint a 
here1    x_2 (no annotation) * 1
1
2
r1cs constraint b 
here1    x_4 (no annotation) * 1
1
i 3
r1cs constraint a 
here    1 * 7291511875036787203678836918395449182321513594297117053754470978267401673272
here1    x_5 (no annotation) * 1
7291511875036787203678836918395449182321513594297117053754470978267401673272
0
r1cs constraint b 
here    1 * 1
1
i 4
r1cs constraint a 
here1    x_2 (no annotation) * 1
1
2
r1cs constraint b 
here1    x_6 (no annotation) * 1
1
i 5
r1cs constraint a 
here    1 * 31673561585488433599050791904540180595868436979789025252305780393337142503
here1    x_7 (no annotation) * 1
31673561585488433599050791904540180595868436979789025252305780393337142503
0
r1cs constraint b 
here    1 * 1
1
i 6
r1cs constraint a 
here1    x_2 (no annotation) * 1
1
2
r1cs constraint b 
here1    x_8 (no annotation) * 1
1
i 7
r1cs constraint a 
here    1 * 973773522213953214195682434770678475784580037887673996447341121400076089101
here1    x_9 (no annotation) * 1
973773522213953214195682434770678475784580037887673996447341121400076089101
0
r1cs constraint b 
here    1 * 1
1

    */
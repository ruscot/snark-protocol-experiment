/**
 * @brief Evaluate a R1CS cosntraint with the current variable assignment
 * Recall an R1CS constraint is as follow 
 * A * B = C
 * @tparam FieldT Field for the constraint
 * @param a Our A
 * @param b Our B
 * @param assignment 
 * @return FieldT corresponding to the value of C 
 */
template<typename FieldT>
FieldT evaluation_on_linear_combination(linear_combination<FieldT> a, linear_combination<FieldT> b, std::vector<FieldT> &assignment) {
    FieldT elemA = a.evaluate(assignment);
    FieldT elemB = b.evaluate(assignment);
    FieldT elemC = elemA * elemB;
    return elemC;
}

/**
 * @brief Just a function to check our computation with the R1CS. Compute the polynomial on "x_value" with the horner method
 * 
 * @tparam FieldT The field of the polynomial
 * @param poly The polynomial coefficient
 * @param degree Degree of the polynomial 
 * @param x_value The value of x
 * @return libff::Fr<FieldT> The result of our computation
 */
template<typename FieldT>
libff::Fr<FieldT> evaluation_polynomial_horner(vector<libff::Fr<FieldT>> poly, uint64_t degree, libff::Fr<FieldT> x_value) {
    libff::Fr<FieldT> res = poly[degree];
    for(uint64_t i = degree; i > 0; i-=1){
        res = res*x_value + poly[i-1];
    }
    return res;
}
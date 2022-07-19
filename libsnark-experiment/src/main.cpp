#include "snark_polynomial_in_clear.hpp"

int main(int argc, char * argv[])
{
    if (argc < 3) {
	    printf("Usage: main <Polynomial degree> <Number of try for timing> \n");
	    return 1;
    }

    //The degree of our polynomial
    const uint64_t polynomial_degree = atoi(argv[1]) - 1;
    //The number of try of our protocol (to make a mean of our timing)
    const int number_of_try = atoi(argv[2]);
    test_polynomial_in_clear(polynomial_degree, number_of_try);
    
    return 0;
}
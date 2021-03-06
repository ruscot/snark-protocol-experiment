#include "snark_polynomial_in_clear.hpp"
#include <unistd.h>
#include <fstream> 
#include <iostream>
#include <stdio.h>
#include <string>
#include <stdlib.h>


int main(int argc, char * argv[])
{
    if (argc < 4) {
	    printf("Usage: main <Polynomial degree> <Number of try for timing> <0 to launch only test 1 otherwise>\n");
	    return 1;
    }

    //The degree of our polynomial
    const uint64_t polynomial_degree = atoi(argv[1]) - 1;
    //The number of try of our protocol (to make a mean of our timing)
    const int number_of_try = atoi(argv[2]);
    const int launch_only_test = atoi(argv[3]);
    if(launch_only_test == 0){
        printf("Launch test on our protocol\n");
        printf("Test the update part of our protocol on the index 0");
        int old_stdout = dup(1);
        fclose(stdout);
        try{
            //Try to do another update to see if it works too
            test_update_index_0(polynomial_degree);
            stdout = fdopen(old_stdout, "w");
            printf(".........OK\n\n");
        }catch(std::runtime_error& e) {
            stdout = fdopen(old_stdout, "w");
            string error_msg = e.what();
            printf(".........NOT OK\n");
            printf("\tError message : %s\n\n", error_msg.c_str());
        }
        old_stdout = dup(fileno(stdout));
        printf("Test the update part of our protocol on the index 1");
        fclose(stdout);
        try{
            //Try to do another update to see if it works too
            test_update_index_1(polynomial_degree);
            stdout = fdopen(old_stdout, "w");
            printf(".........OK\n\n");
        }catch(std::runtime_error& e) {
            stdout = fdopen(old_stdout, "w");
            string error_msg = e.what();
            printf(".........NOT OK\n");
            printf("\tError message : %s\n\n", error_msg.c_str());
        }
        printf("We'll do 10 test with random index to update\n\n"); 
        int random_index_to_update;
        srand (time(NULL));
        for(int i = 0; i < 10; i++){
            old_stdout = dup(fileno(stdout));
            random_index_to_update = rand() % (polynomial_degree-1) + 2;
            printf("Test the update part of our protocol with the random index \"%d\" ", random_index_to_update);
            fclose(stdout);
            try{
                //Try to do another update to see if it works too
                test_update_random_index(polynomial_degree, random_index_to_update);
                stdout = fdopen(old_stdout, "w");
                printf(".........OK\n\n");
            }catch(std::runtime_error& e) {
                stdout = fdopen(old_stdout, "w");
                string error_msg = e.what();
                printf(".........NOT OK\n");
                printf("\tError message : %s\n\n", error_msg.c_str());
            }   
        }
        
    } else {
        test_polynomial_in_clear(polynomial_degree, number_of_try);
    }
    

    return 0;
}
/**
 * @file main.cpp
 * @author Martinez Anthony
 * @brief Main program to run our protocol or the tests
 * @version 0.1
 * @date 2022-07-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "snark_polynomial_in_clear.hpp"
#include "snark_tests.hpp"
#include <unistd.h>
#include <fstream> 
#include <iostream>
#include <stdio.h>
#include <string>
#include <stdlib.h>

int main(int argc, char * argv[]) {
    if (argc < 4) {
	    printf("Usage: main <Polynomial degree> <Number of try for timing> <0 to launch only tests 1 otherwise> <1 if you want to print program output during tests (we'll don't print by default)>\n");
	    return 1;
    }

    //The degree of our polynomial
    uint64_t polynomial_degree = atoi(argv[1]) - 1;
    //The number of try of our protocol (to make a mean of our timing)
    int number_of_try = atoi(argv[2]);
    int launch_only_test = atoi(argv[3]);
    int program_output_during_test = 0;
    if (argc == 5) {
	    program_output_during_test = atoi(argv[4]);
    } 
    if(launch_only_test == 0 && program_output_during_test == 0) {
        int number_of_test_failed = 0;
        printf("Launch test on our protocol\n");
        printf("Test the update part of our protocol on the index 0");
        int old_stdout = dup(1);
        fclose(stdout);
        try {
            //Try to do another update to see if it works too
            test_update_index_0(polynomial_degree);
            stdout = fdopen(old_stdout, "w");
            printf("\t\t\t\t\t\t.........PASS\n\n");
        } catch(std::runtime_error& e) {
            stdout = fdopen(old_stdout, "w");
            string error_msg = e.what();
            printf("\t\t\t\t\t\t.........FAILED\n");
            number_of_test_failed++;
            printf("\tError message : %s\n\n", error_msg.c_str());
        }
        old_stdout = dup(fileno(stdout));
        printf("Test the update part of our protocol on the index 1");
        fclose(stdout);
        try {
            //Try to do another update to see if it works too
            test_update_index_1(polynomial_degree);
            stdout = fdopen(old_stdout, "w");
            printf("\t\t\t\t\t\t.........PASS\n\n");
        } catch(std::runtime_error& e) {
            stdout = fdopen(old_stdout, "w");
            string error_msg = e.what();
            printf("\t\t\t\t\t\t.........FAILED\n");
            number_of_test_failed++;
            printf("\tError message : %s\n\n", error_msg.c_str());
        }
        printf("We'll do 10 test with random index to update to see if we don't get any error\n\n"); 
        int random_index_to_update;
        srand (time(NULL));
        for(int i = 0; i < 10; i++) {
            old_stdout = dup(fileno(stdout));
            random_index_to_update = rand() % (polynomial_degree-2) + 2;
            if(random_index_to_update < 10) {
                printf("\tCoefficient index to update \"0%d\" ", random_index_to_update);
            } else {
                printf("\tCoefficient index to update \"%d\" ", random_index_to_update);
            }
            
            fclose(stdout);
            try {
                //Try to do another update to see if it works too
                test_update_random_index(polynomial_degree, random_index_to_update);
                stdout = fdopen(old_stdout, "w");
                printf("\t\t\t\t\t\t\t.........PASS\n\n");
            } catch(std::runtime_error& e) {
                stdout = fdopen(old_stdout, "w");
                string error_msg = e.what();
                printf("\t\t\t\t\t\t\t.........FAILED\n");
                number_of_test_failed++;
                printf("\tError message : %s\n\n", error_msg.c_str());
            }   
        }
        
        old_stdout = dup(fileno(stdout));
        printf("Test that we raised an error if the R1CS is not correct"); 
        fclose(stdout);
        try {
            //Try to do another update to see if it works too
            test_raise_error_for_wrong_r1cs_construction(polynomial_degree);
            stdout = fdopen(old_stdout, "w");
            printf("\t\t\t\t\t\t.........FAILED\n\n");
            number_of_test_failed++;
        } catch(std::runtime_error& e) {
            stdout = fdopen(old_stdout, "w");
            printf("\t\t\t\t\t\t.........PASS\n\n");
        }   

        old_stdout = dup(fileno(stdout));
        printf("Test that we raised an error if the R1CS is not correctly updated for coefficient 0"); 
        fclose(stdout);
        try {
            //Try to do another update to see if it works too
            test_raise_error_for_wrong_r1cs_update(polynomial_degree, 0);
            stdout = fdopen(old_stdout, "w");
            printf("\t\t.........FAILED\n\n");
            number_of_test_failed++;
        } catch(std::runtime_error& e) {
            stdout = fdopen(old_stdout, "w");
            printf("\t\t.........PASS\n\n");
        }   

        old_stdout = dup(fileno(stdout));
        printf("Test that we raised an error if the R1CS is not correctly updated for coefficient 1"); 
        fclose(stdout);
        try {
            //Try to do another update to see if it works too
            test_raise_error_for_wrong_r1cs_update(polynomial_degree, 1);
            stdout = fdopen(old_stdout, "w");
            printf("\t\t.........FAILED\n\n");
            number_of_test_failed++;
        } catch(std::runtime_error& e) {
            stdout = fdopen(old_stdout, "w");
            printf("\t\t.........PASS\n\n");
        }


        printf("We'll do 10 test with random index to update and see if we alway raised an error when the R1CS is wrongly updated\n\n"); 
        for(int i = 0; i < 10; i++) {
            old_stdout = dup(fileno(stdout));
            random_index_to_update = rand() % (polynomial_degree-2) + 2;
            if(random_index_to_update < 10) {
                printf("\tCoefficient index to update \"0%d\" ", random_index_to_update);
            } else {
                printf("\tCoefficient index to update \"%d\" ", random_index_to_update);
            }
            
            fclose(stdout);
            try {
                //Try to do another update to see if it works too
                test_raise_error_for_wrong_r1cs_update(polynomial_degree, random_index_to_update);
                stdout = fdopen(old_stdout, "w");
                printf("\t\t\t\t\t\t\t.........FAILED\n\n");
                number_of_test_failed++;
            } catch(std::runtime_error& e) {
                stdout = fdopen(old_stdout, "w");
                printf("\t\t\t\t\t\t\t.........PASS\n\n");
            } 
        }

        old_stdout = dup(fileno(stdout));
        printf("Test that we don't raised an error if the keys is not correctly updated for coefficient 0"); 
        fclose(stdout);
        try {
            //Try to do another update to see if it works too
            test_raise_error_for_wrong_keys_update(polynomial_degree, 0);
            stdout = fdopen(old_stdout, "w");
            printf("\t.........PASS\n\n");
        } catch(std::runtime_error& e) {
            stdout = fdopen(old_stdout, "w");
            string error_msg = e.what();
            printf("\t.........FAILED\n\n");
            number_of_test_failed++;
            printf("\tError message : %s\n\n", error_msg.c_str());
        }   

        old_stdout = dup(fileno(stdout));
        printf("Test that we don't raised an error if the keys is not correctly updated for coefficient 1");
        fclose(stdout);
        try {
            //Try to do another update to see if it works too
            test_raise_error_for_wrong_keys_update(polynomial_degree, 1);
            stdout = fdopen(old_stdout, "w");
            printf("\t.........PASS\n\n");
        } catch(std::runtime_error& e) {
            stdout = fdopen(old_stdout, "w");
            string error_msg = e.what();
            printf("\t.........FAILED\n\n");
            printf("\tError message : %s\n\n", error_msg.c_str());
            number_of_test_failed++;
        }


        printf("We'll do 10 test with random index to update and see if we alway raised an error when the keys is wrongly updated\n\n"); 
        for(int i = 0; i < 10; i++) {
            old_stdout = dup(fileno(stdout));
            random_index_to_update = rand() % (polynomial_degree-2) + 2;
            if(random_index_to_update < 10) {
                printf("\tCoefficient index to update \"0%d\" ", random_index_to_update);
            } else {
                printf("\tCoefficient index to update \"%d\" ", random_index_to_update);
            }
            
            fclose(stdout);
            try {
                //Try to do another update to see if it works too
                test_raise_error_for_wrong_keys_update(polynomial_degree, random_index_to_update);
                stdout = fdopen(old_stdout, "w");
                printf("\t\t\t\t\t\t\t.........FAILED\n\n");
                number_of_test_failed++;
            } catch(std::runtime_error& e) {
                stdout = fdopen(old_stdout, "w");
                printf("\t\t\t\t\t\t\t.........PASS\n\n");
            } 
        }

        printf("We'll try to update 100 random coefficient on the same polynomial of degree 10 and see if everything is correct\n\n");
        vector<int> index_to_update_list;
        uint64_t current_polynomial_degree = 10;
        old_stdout = dup(fileno(stdout));
        for(int i = 0; i < 100; i++) {
            random_index_to_update = rand() % (current_polynomial_degree-2) + 2;
            if(random_index_to_update < 10) {
                printf("\tCoefficient index to update \"0%d\" \n", random_index_to_update);
            } else {
                printf("\tCoefficient index to update \"%d\" \n", random_index_to_update);
            }
            index_to_update_list.push_back(random_index_to_update);
        }
        fclose(stdout);
        try {
            //Try to do another update to see if it works too
            test_update_multiple_index(current_polynomial_degree, index_to_update_list);
            stdout = fdopen(old_stdout, "w");
            printf("\t\t\t\t\t\t\t.........PASS\n\n");
        } catch(std::runtime_error& e) {
            stdout = fdopen(old_stdout, "w");
            string error_msg = e.what();
            printf("\t\t\t\t\t\t\t.........FAILED\n\n");
            printf("\tError message : %s\n\n", error_msg.c_str());
            number_of_test_failed++;
        }

        if(number_of_test_failed == 0) {
            printf("All tests pass\n");
        } else {
            printf("%d tests failed\n", number_of_test_failed);
        }
    } else if (launch_only_test == 0) {
        int number_of_test_failed = 0;
        printf("Launch test on our protocol\n");
        printf("Test the update part of our protocol on the index 0");
        try {
            //Try to do another update to see if it works too
            test_update_index_0(polynomial_degree);
            printf("\t\t\t\t\t\t.........PASS\n\n");
        } catch(std::runtime_error& e) {
            string error_msg = e.what();
            printf("\t\t\t\t\t\t.........FAILED\n");
            number_of_test_failed++;
            printf("\tError message : %s\n\n", error_msg.c_str());
        }
        printf("Test the update part of our protocol on the index 1");
        try {
            //Try to do another update to see if it works too
            test_update_index_1(polynomial_degree);
            printf("\t\t\t\t\t\t.........PASS\n\n");
        } catch(std::runtime_error& e) {
            string error_msg = e.what();
            printf("\t\t\t\t\t\t.........FAILED\n");
            number_of_test_failed++;
            printf("\tError message : %s\n\n", error_msg.c_str());
        }
        printf("We'll do 10 test with random index to update to see if we don't get any error\n\n"); 
        int random_index_to_update;
        srand (time(NULL));
        for(int i = 0; i < 10; i++) {
            random_index_to_update = rand() % (polynomial_degree-2) + 2;
            if(random_index_to_update < 10) {
                printf("\tCoefficient index to update \"0%d\" ", random_index_to_update);
            } else {
                printf("\tCoefficient index to update \"%d\" ", random_index_to_update);
            }
            
            try {
                //Try to do another update to see if it works too
                test_update_random_index(polynomial_degree, random_index_to_update);
                printf("\t\t\t\t\t\t\t.........PASS\n\n");
            } catch(std::runtime_error& e) {
                string error_msg = e.what();
                printf("\t\t\t\t\t\t\t.........FAILED\n");
                number_of_test_failed++;
                printf("\tError message : %s\n\n", error_msg.c_str());
            }   
        }
        
        printf("Test that we raised an error if the R1CS is not correct"); 
        try {
            //Try to do another update to see if it works too
            test_raise_error_for_wrong_r1cs_construction(polynomial_degree);
            printf("\t\t\t\t\t\t.........FAILED\n\n");
            number_of_test_failed++;
        } catch(std::runtime_error& e) {
            printf("\t\t\t\t\t\t.........PASS\n\n");
        }
        printf("Test that we raised an error if the R1CS is not correctly updated for coefficient 0"); 
        try {
            //Try to do another update to see if it works too
            test_raise_error_for_wrong_r1cs_update(polynomial_degree, 0);
            printf("\t\t.........FAILED\n\n");
            number_of_test_failed++;
        } catch(std::runtime_error& e) {
            printf("\t\t.........PASS\n\n");
        }   
        printf("Test that we raised an error if the R1CS is not correctly updated for coefficient 1"); 
        try {
            //Try to do another update to see if it works too
            test_raise_error_for_wrong_r1cs_update(polynomial_degree, 1);
            printf("\t\t.........FAILED\n\n");
            number_of_test_failed++;
        } catch(std::runtime_error& e) {
            printf("\t\t.........PASS\n\n");
        }


        printf("We'll do 10 test with random index to update and see if we alway raised an error when the R1CS is wrongly updated\n\n"); 
        for(int i = 0; i < 10; i++) {
            random_index_to_update = rand() % (polynomial_degree-2) + 2;
            if(random_index_to_update < 10) {
                printf("\tCoefficient index to update \"0%d\" ", random_index_to_update);
            } else {
                printf("\tCoefficient index to update \"%d\" ", random_index_to_update);
            }
            try {
                //Try to do another update to see if it works too
                test_raise_error_for_wrong_r1cs_update(polynomial_degree, random_index_to_update);
                printf("\t\t\t\t\t\t\t.........FAILED\n\n");
                number_of_test_failed++;
            } catch(std::runtime_error& e) {
                printf("\t\t\t\t\t\t\t.........PASS\n\n");
            } 
        }
        printf("Test that we don't raised an error if the keys is not correctly updated for coefficient 0"); 
        try {
            //Try to do another update to see if it works too
            test_raise_error_for_wrong_keys_update(polynomial_degree, 0);
            printf("\t.........PASS\n\n");
        } catch(std::runtime_error& e) {
            string error_msg = e.what();
            printf("\t.........FAILED\n\n");
            printf("\tError message : %s\n\n", error_msg.c_str());
            number_of_test_failed++;
        }
        printf("Test that we don't raised an error if the keys is not correctly updated for coefficient 1");
        try {
            //Try to do another update to see if it works too
            test_raise_error_for_wrong_keys_update(polynomial_degree, 1);
            printf("\t.........PASS\n\n");
        } catch(std::runtime_error& e) {
            string error_msg = e.what();
            printf("\t.........FAILED\n\n");
            printf("\tError message : %s\n\n", error_msg.c_str());
            number_of_test_failed++;
        }
        printf("We'll do 10 test with random index to update and see if we alway raised an error when the keys is wrongly updated\n\n"); 
        for(int i = 0; i < 10; i++) {
            random_index_to_update = rand() % (polynomial_degree-2) + 2;
            if(random_index_to_update < 10) {
                printf("\tCoefficient index to update \"0%d\" ", random_index_to_update);
            } else {
                printf("\tCoefficient index to update \"%d\" ", random_index_to_update);
            }
            try {
                //Try to do another update to see if it works too
                test_raise_error_for_wrong_keys_update(polynomial_degree, random_index_to_update);
                printf("\t\t\t\t\t\t\t.........FAILED\n\n");
                number_of_test_failed++;
            } catch(std::runtime_error& e) {
                printf("\t\t\t\t\t\t\t.........PASS\n\n");
            } 
        }
        printf("We'll try to update 100 random coefficient on the same polynomial of degree 10 and see if everything is correct\n\n");
        vector<int> index_to_update_list;
        uint64_t current_polynomial_degree = 10;
        for(int i = 0; i < 100; i++) {
            random_index_to_update = rand() % (current_polynomial_degree-2) + 2;
            if(random_index_to_update < 10) {
                printf("\tCoefficient index to update \"0%d\" \n", random_index_to_update);
            } else {
                printf("\tCoefficient index to update \"%d\" \n", random_index_to_update);
            }
            index_to_update_list.push_back(random_index_to_update);
        }
        try {
            //Try to do another update to see if it works too
            test_update_multiple_index(current_polynomial_degree, index_to_update_list);
            printf("\t\t\t\t\t\t\t.........PASS\n\n");
        } catch(std::runtime_error& e) {
            string error_msg = e.what();
            printf("\t\t\t\t\t\t\t.........FAILED\n\n");
            printf("\tError message : %s\n\n", error_msg.c_str());
            number_of_test_failed++;
        }

        if(number_of_test_failed == 0) {
            printf("All tests pass\n");
        } else {
            printf("%d tests failed\n", number_of_test_failed);
        }
    } else {
        test_polynomial_in_clear(polynomial_degree, number_of_try);
    }

    return 0;
}
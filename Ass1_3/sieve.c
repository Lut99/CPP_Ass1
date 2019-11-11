#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "timer.h"

int main(int argc, char *argv[]){
    timer_start();

    // number of natural numbers
    int n = 30000;

    // init list of primes
    long primes[n];
    long primelen = 1;
    primes[0] = 2;

    // iterate over natural numbers
    long nat = 2;
    while (true){
        bool is_prime = true;
        
        // iterate of primes
        for (int i = 0; i < primelen; i++){
            // check if number is devisible by prime
            if (nat % primes[i] == 0) {
                is_prime = false;
                break;
            }
        }

        // add prime to list of primes
        if (is_prime == true){
            primes[primelen] = nat;
            primelen++;

            printf("new prime: %ld\n", nat);
        }
        
        // stop the program early
        if (primelen == n){
            double time = timer_end();
            printf("Took %g seconds\n", time);
            printf("Normalized: %g seconds\n", time / (1. * primelen));
            exit(1);
        }

        // get the next natural number
        nat++;
    }
}
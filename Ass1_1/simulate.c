/*
 * simulate.c
 *
 * Implement your (parallel) simulation here!
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "simulate.h"

typedef struct {
    pthread_t t_id;
    int id;
    int work_size;
} Thread;

/* Add any global variables you may need. */
const double c = 0.15;
double *global_old_array;
double *global_current_array;
double *global_next_array;

/* Add any functions you may need (like a worker) here. */
void *thread_func(void *args) {
    Thread *t = (Thread*) args;

    int t_i = t->id * t->work_size + 1;
    for (int i = t_i; i < t_i + t->work_size; i++) {
        global_next_array[i] = 2 * global_current_array[i] - 
                                global_old_array[i] + c * 
                                (global_current_array[i - 1] - 
                                (2 * global_current_array[i] - 
                                global_current_array[i + 1]));
    }

    return NULL;
}

/*
 * Executes the entire simulation.
 *
 * Implement your code here.
 *
 * i_max: how many data points are on a single wave
 * t_max: how many iterations the simulation should run
 * num_threads: how many threads to use (excluding the main threads)
 * old_array: array of size i_max filled with data for t-1
 * current_array: array of size i_max filled with data for t
 * next_array: array of size i_max. You should fill this with t+1
 */
double *simulate(const int i_max, const int t_max, const int num_threads,
        double *old_array, double *current_array, double *next_array)
{
    /* Simulate a fucking wave */
    int t, i, work_size;
    double *temp;
    Thread *threads[num_threads];

    if (num_threads == 0) {
        work_size = 0;
    } else {
        work_size = (i_max - 2) / num_threads;
    }

    // Assign the globals
    global_old_array = old_array;
    global_current_array = current_array;
    global_next_array = next_array;

    for (t = 0; t < t_max; t++) {
        // Create n_threads threads
        for (i = 0; i < num_threads; i++) {
            // Create the threads
            threads[i] = (Thread*) malloc(sizeof(Thread));
            threads[i]->id = i;
            threads[i]->work_size = work_size;
            pthread_create(&threads[i]->t_id, NULL, &thread_func, (void*) threads[i]);
        }
        // Do the leftover work
        for (i = num_threads * work_size; i < i_max; i++) {
            global_next_array[i] = 2 * global_current_array[i] - 
                                   global_old_array[i] + c * 
                                   (global_current_array[i - 1] - 
                                   (2 * global_current_array[i] - 
                                   global_current_array[i + 1]));
        }
        // Reap all threads now that we've done the overflowing items
        for (i = 0; i < num_threads; i++) {
            pthread_join(threads[i]->t_id, NULL);
            // Clean up
            free(threads[i]);
        }

        // Swap the arrays for the next iteration
        temp = global_old_array;
        global_old_array = global_current_array;
        global_current_array = global_next_array;
        global_next_array = temp;
    }

    /* You should return a pointer to the array with the final results. */
    return current_array;
}

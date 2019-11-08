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
    int start_i;
    int stop_i;
    int work_time;
    double *old_array;
    double *current_array;
    double *next_array;
    pthread_barrier_t *barrier;
} Thread;

/* Add any global variables you may need. */
const double c = 0.15;

/* Add any functions you may need (like a worker) here. */
void *thread_func(void *args) {
    Thread *trd = (Thread*) args;
    double *temp, *old_array, *current_array, *next_array;
    int t, i;
    int t_max, start_i, stop_i;

    // Initialize private variables
    start_i = trd->start_i;
    stop_i = trd->stop_i;
    t_max = trd->work_time;
    old_array = trd->old_array;
    current_array = trd->current_array;
    next_array = trd->next_array;

    // Do the timesteps
    for (t = 0; t < t_max; t++) {
        //printf("--------\nThread %i did a new timestep %i\n--------\n", trd->start_i, t);
        //fflush(stdout);
        for (i = start_i; i < stop_i; i++) {
            next_array[i] = 2 * current_array[i] - 
                            old_array[i] + c * 
                            (current_array[i - 1] - 
                            (2 * current_array[i] - 
                            current_array[i + 1]));
            //printf("Thread %i did a spatial step (new value: %f)\n", trd->start_i, next_array[i]);
            //fflush(stdout);
        }

        // Swap the buffers
        temp = old_array;
        old_array = current_array;
        current_array = next_array;
        next_array = temp;
        
        // Wait until the other threads are done with this timestep
        pthread_barrier_wait(trd->barrier);
    }
    
    // Before returning, save the current_array as result
    trd->current_array = current_array;

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

    /* INITIALIZATION PHASE */

    int i, work_size;
    Thread *threads;
    pthread_barrier_t swap_barrier;

    // Divide the work (with DivideByZero protection)
    if (num_threads == 0) {
        work_size = 0;
    } else {
        work_size = (i_max - 2) / num_threads;
    }

    // Declare the barrier
    pthread_barrier_init(&swap_barrier, NULL, num_threads);

    // Allocate the arguments array
    threads = (Thread*) malloc(num_threads * sizeof(Thread));

    // Create n_threads threads
    for (i = 0; i < num_threads; i++) {
        // Create the argument struct
        threads[i].start_i = (work_size * i) + 1;
        threads[i].stop_i = work_size * (i + 1);
        threads[i].work_time = t_max;
        threads[i].old_array = old_array;
        threads[i].current_array = current_array;
        threads[i].next_array = next_array;
        threads[i].barrier = &swap_barrier;
        if (i == num_threads - 1) {
            // Instead of the standard worksize, assign the rest of the timestep
            threads[i].stop_i = i_max - 2;
        }
        // Use it to create the thread
        pthread_create(&threads[i].t_id, NULL, &thread_func, (void*) &threads[i]);
    }

    /* CLEANUP PHASE */

    // Reap all threads now that we're done with the last loop
    double *result = current_array;
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i].t_id, NULL);
        result = threads[i].current_array;
    }
    
    // Destroy the synchronisation barrier
    pthread_barrier_destroy(&swap_barrier);

    // Destroy the argument array
    free(threads);

    /* You should return a pointer to the array with the final results. */
    return result;
}

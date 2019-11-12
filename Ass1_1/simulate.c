/*
 * simulate.c
 *
 * Implement your (parallel) simulation here!
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "simulate.h"

// Determines the status of the thread.
//   0 = busy with timestep
//   1 = waiting for other threads
//   2 = signal from main threads that they can continue
//   3 = the thread is done
#define THREAD_BUSY 0
#define THREAD_WAITING 1
#define THREAD_CONTINUE 2
#define THREAD_DONE 3

typedef struct {
    pthread_t t_id;
    int start_i;
    int stop_i;
    int work_time;
    double *old_array;
    double *current_array;
    double *next_array;
    // Declare stuff for the "barrier"
    pthread_mutex_t *lock;
    pthread_cond_t *can_continue;
    char status;
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

    // Initialize barrier related variables
    pthread_mutex_t *lock = trd->lock;
    pthread_cond_t *can_continue = trd->can_continue;

    // Do the timesteps
    for (t = 0; t < t_max; t++) {
        for (i = start_i; i < stop_i; i++) {
            next_array[i] = 2 * current_array[i] - 
                            old_array[i] + c * 
                            (current_array[i - 1] - 
                            (2 * current_array[i] - 
                            current_array[i + 1]));
        }

        // Swap the buffers
        temp = old_array;
        old_array = current_array;
        current_array = next_array;
        next_array = temp;
        
        // Now that we're done, set own status to waiting and sleep until the
        //   main thread signals we can continue
        pthread_mutex_lock(lock);
        trd->status = THREAD_WAITING;
        while (trd->status != THREAD_CONTINUE) {
            pthread_cond_wait(can_continue, lock);
        }
        trd->status = THREAD_BUSY;
        pthread_mutex_unlock(lock);
    }
    
    // Before returning, save the current_array as result
    trd->status = THREAD_DONE;
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

    int i, work_size, threads_done, threads_waiting;
    Thread *threads;
    pthread_mutex_t lock;
    pthread_cond_t signal;

    // Divide the work (with DivideByZero protection)
    if (num_threads == 0) {
        work_size = 0;
    } else {
        work_size = (i_max - 2) / num_threads;
    }

    // Init the lock & counter
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&signal, NULL);

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
        if (i == num_threads - 1) {
            // Instead of the standard worksize, assign the rest of the timestep
            threads[i].stop_i = i_max - 1;
        }

        // Pass stuff for the barrier
        threads[i].lock = &lock;
        threads[i].can_continue = &signal;
        threads[i].status = 0;

        // Use it to create the thread
        pthread_create(&threads[i].t_id, NULL, &thread_func, (void*) &threads[i]);
    }

    /* RUN PHASE */

    // Police the threads so that they wait for each other
    while (1) {
        threads_done = 0;
        threads_waiting = 0;
        // Count how many threads are waiting and how many are done
        for (i = 0; i < num_threads; i++) {
            if (threads[i].status == THREAD_WAITING) {
                threads_waiting++;
            } else if (threads[i].status == THREAD_DONE) {
                threads_done++;
            }
        }
        if (threads_waiting == num_threads) {
            // In case they're all waiting, allow them to continue
            for (i = 0; i < num_threads; i++) {
                threads[i].status = THREAD_CONTINUE;
            }
            pthread_mutex_lock(&lock);
            pthread_cond_broadcast(&signal);
            pthread_mutex_unlock(&lock);
        } else if (threads_done == num_threads) {
            // Otherwise, if they're all done, stop
            break;
        }
    }

    /* CLEANUP PHASE */

    // Reap all threads now that we're done with the last loop
    double *result = current_array;
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i].t_id, NULL);
        result = threads[i].current_array;
    }
    
    // Destroy the synchronisation barrier
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&signal);

    // Destroy the argument array
    free(threads);

    /* You should return a pointer to the array with the final results. */
    return result;
}

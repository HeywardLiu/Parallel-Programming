#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t mutex_sum;

typedef struct  // global variable shared by all threads.
{
    long long int loop_times;
    long long int *num_in_circle; // Used to access the global value of the number of hit 
} args_t; // The arguments pass to pthread_create() function


void* estimatePI(void *args) {  // Estimate PI via Monte-Carlo method 
    // parse args
    args_t *arg = (args_t *)args;
    long long int loop_times = arg->loop_times;
    long long int *num_in_circle = arg->num_in_circle; // global variable shared between threads 

    long long int local_num_in_circle = 0;
    unsigned int seed = 777;  // notify rand_r() those threads created from the same program.
    
    for (long long int i=0; i < loop_times; i++) {  
        // Generate random floating number between -1 to 1
        double x = ((double) rand_r(&seed) / RAND_MAX) * 2.0 - 1.0;
        double y = ((double) rand_r(&seed) / RAND_MAX) * 2.0 - 1.0;

        double dist = x*x + y*y;
        if (dist <= 1.0)
            local_num_in_circle++;
    }

    pthread_mutex_lock(&mutex_sum);
    *num_in_circle += local_num_in_circle;
    pthread_mutex_unlock(&mutex_sum);

    pthread_exit((void *)0);
}

int main(int argc, char** argv) {
    // parse arguments
    const unsigned int num_of_thread = atoi(argv[1]);
    long long int num_of_tosses = atoll(argv[2]);

    // prepare thread parameters
    const long long int partition_size = (num_of_tosses / num_of_thread);
    pthread_t threads[num_of_thread];
    args_t arg[num_of_thread];

    pthread_mutex_init(&mutex_sum, NULL);
    
    // Set pthread attribute and make pthread joinable
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // global variable to sum up local_num_in_circles of each threads
    long long int *num_in_circle = (long long int *) malloc(sizeof(*num_in_circle));  
    *num_in_circle = 0; 

    for (int i=0; i<num_of_thread; i++) {
        // Set argueemnts to each thread 
        arg[i].loop_times = partition_size;    // local variable
        arg[i].num_in_circle = num_in_circle;  // global variable

         // Create a new thread and run function with correspoding arg[i] arguments
        pthread_create(&threads[i], &attr, estimatePI, (void *) &arg[i]);
    }
    
    /* Free attribute*/
    pthread_attr_destroy(&attr);
    
    /* Wait for the other threads*/
    void *status;
    for (int i = 0;i < num_of_thread;i++) {
        pthread_join(threads[i], &status);
    }

    pthread_mutex_destroy(&mutex_sum);

    double pi = 4 * ((*num_in_circle) / (double)num_of_tosses);
    printf("%.7lf\n", pi);

    return 0;
}
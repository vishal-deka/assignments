/*
The solution is to use the brute force algorithm to count inverses
but parallelize the outer loop. The outer loop iterates over the 
array once and so it is divided fairly among the threads.
*/


#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <pthread.h>
#include <ctime>        // std::time
#include <cstdlib>      // std::rand, std::srand
#include <iostream>
#include <cmath>

void *print_message_function(void *ptr);

int *arr;
int *counts;
int SIZE,NUM_THREADS;
int main(int argc, char *argv[])
{
    SIZE= atoi(argv[1]);
    NUM_THREADS = atoi(argv[2]);
    if (NUM_THREADS>SIZE) {
        std::cout<<"ERROR: Number of threads must be less than size of array\n";
        return 0;
    }
    else if (SIZE<=0 || NUM_THREADS<=0)
    {
        std::cout<<"Invalid values of n and p\n";
        return 0;
    }

    pthread_t threads[NUM_THREADS];

    int retval;
    int thread_args[NUM_THREADS];
    std::cout<<"size: "<<SIZE<<" threads: "<<NUM_THREADS<<std::endl;
    
    // generate array
    arr = (int *)malloc(SIZE * sizeof(int));
    for(int i = 0; i < SIZE; i++){
            arr[i] = i;
        }
    
    //shuffle the array
    std::srand ( unsigned ( std::time(0) ) );
    std::random_shuffle(arr, arr+SIZE);
    std::cout<<"Array after shuffling: \n";
    for(int i = 0; i < SIZE; i++){
        std::cout<<arr[i]<<",";
    }
    std::cout<<"\n";

    // output array
    counts = (int *)malloc(NUM_THREADS * sizeof(int));
    
    //create threads
    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_args[i] = i;

        retval = pthread_create(&threads[i], NULL, print_message_function, (void *)&thread_args[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        // second argument is a buffer for return value or 0.
        retval = pthread_join(threads[i], 0);
    }
    int sum = 0;
    for (int i = 0; i < NUM_THREADS; i++)
        sum += counts[i];
    printf("Inversions: %d\n", sum);

    return 0;
}

void *print_message_function(void *ptr)
{
    int countloc = 0, a_ = 0;
    int id = *((int *)ptr);
    // calculate offset and chunk size for this thread from id
    int offset = id * ceil((float)SIZE / NUM_THREADS);
    int chunk = ceil((float)SIZE/NUM_THREADS);

    if (id == NUM_THREADS - 1){
        // for the last iteration we iterate till the 2nd last element in sequential code
        a_ = -1;
        // the last thread will get the remaining elements after equal distribution
        chunk -= NUM_THREADS*chunk - SIZE;
    }
    // std::cout<<"id: "<<id << " chunk size: " <<chunk <<std::endl;
    for (int i = offset; i < offset + chunk + a_; i++)
    {
        for (int j = i + 1; j < SIZE; j++)
            if (arr[i] > arr[j])
                countloc++;
    }
    // printf("\n");
    counts[id] = countloc;
}
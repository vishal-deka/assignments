/*
The solution is to use the brute force algorithm to count inverses
but parallelize the outer loop. The outer loop iterates over the
array once and so it is divided fairly among the threads.
*/

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <pthread.h>
#include <ctime>   // std::time
#include <cstdlib> // std::rand, std::srand
#include <iostream>
#include <cmath>
#include <random>
#define ENDL std::endl

void *inversion_counter(void *ptr);

// Generate " max " random numbers in the range [0 , max -1] and
// fill in the output variable " array "
void generate_rand_nums(int max, int *array)
{
    const int range_from = 0;
    const int range_to = max - 1;
    std ::random_device rd;                                          // obtain a random number from hardware
    std ::mt19937 gen(rd());                                         // seed the generator
    std ::uniform_int_distribution<int> distr(range_from, range_to); // inclusive
    for (int i = 0; i < max; i++)
    {
        array[i] = distr(gen); // generate numbers
    }
}

int *arr;
int *counts;
int SIZE, NUM_THREADS;
int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        printf("Usage: ./prob2 n p\n");
        return 0;
    }
    SIZE = atoi(argv[1]);
    NUM_THREADS = atoi(argv[2]);
    if (NUM_THREADS > SIZE)
    {
        std::cout << "ERROR: Number of threads must be less than size of array\n";
        return 0;
    }
    else if (SIZE <= 0 || NUM_THREADS <= 0)
    {
        std::cout << "Invalid values of n and p\n";
        return 0;
    }

    pthread_t threads[NUM_THREADS];

    int retval;
    int thread_args[NUM_THREADS];
    //std::cout << "size: " << SIZE << " threads: " << NUM_THREADS << std::endl;

    // generate random array
    arr = (int *)malloc(SIZE * sizeof(int));
    generate_rand_nums(SIZE, arr);
    // for(int i=0;i<SIZE;i++) std::cout<<arr[i]<<",";
    // std::cout<<ENDL;
    
    // array for results of each thread
    counts = (int *)malloc(NUM_THREADS * sizeof(int));

    // create threads
    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_args[i] = i;

        retval = pthread_create(&threads[i], NULL, inversion_counter, (void *)&thread_args[i]);
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

void *inversion_counter(void *ptr)
{
    int countloc = 0, a_ = 0;
    int id = *((int *)ptr);
    // calculate offset and chunk size for this thread from id
    int offset = id * ceil((float)SIZE / NUM_THREADS);
    int chunk = ceil((float)SIZE / NUM_THREADS);

    if (id == NUM_THREADS - 1)
    {
        // for the last iteration we iterate till the 2nd last element in sequential code
        a_ = -1;
        // the last thread will get the remaining elements after equal distribution
        chunk -= NUM_THREADS * chunk - SIZE;
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
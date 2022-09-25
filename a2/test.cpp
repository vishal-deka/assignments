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
#include <fstream>
#include <string>
void *read_file(void *ptr);

int *arr;
int *counts;
int SIZE, NUM_THREADS;
std::ifstream is;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
int main(int argc, char *argv[])
{
    NUM_THREADS = 4;
    pthread_t threads[NUM_THREADS];

    int retval;
    int thread_args[NUM_THREADS];
    is.open("file.txt");

    // create threads
    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_args[i] = i;

        retval = pthread_create(&threads[i], NULL, read_file, (void *)&thread_args[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        // second argument is a buffer for return value or 0.
        retval = pthread_join(threads[i], 0);
    }

    return 0;
}

void *read_file(void *ptr)
{
    while (!is.eof())
    {
        pthread_mutex_lock(&m);
        printf("thread\n");
        std::string line;
        std::getline(is, line);
        std::cout << line << std::endl;

        pthread_mutex_unlock(&m);
    }
}
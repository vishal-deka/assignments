#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <iostream>
#include<algorithm>
/* Producer/consumer program illustrating conditional variables */

/* Size of shared buffer */


int *buffer; /* shared buffer */
int BUF_SIZE;
int add = 0;          /* place to add next element */
int rem = 0;          /* place to remove next element */
int num = 0;          /* number elements in buffer */
int done = 0;
int totprod = 0, totcon = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;    /* mutex lock for buffer */
pthread_mutex_t n = PTHREAD_MUTEX_INITIALIZER;  // mutex for file
pthread_cond_t c_cons = PTHREAD_COND_INITIALIZER; /* consumer waits on this cond var */
pthread_cond_t c_prod = PTHREAD_COND_INITIALIZER; /* producer waits on this cond var */
int flag = 0;
void *producer(void *param);
void *consumer(void *param);
std::ifstream is;
std::ofstream os;
int NUM_THREADS;
int P,W;
std::vector<int> nos;
int eof=0;

int distinct_elements(std::vector<int> arr){
   // Sorting the array
   int n = arr.size();
   std::sort(arr.begin(),arr.end());
   // Traverse the sorted array
   int count = 0;
   for (int i = 0; i < n; i++){
      // Moving the index when duplicate is found
      while (i < n - 1 && arr[i] == arr[i + 1]){
        std::cout<<arr[i]<<" ";
         i++;
      }
      count++;
   }
std::cout<<"\n";
   return count;
}

bool isPrime(int n)
{
    // Corner case
    if (n <= 1)
        return false;

    // Check from 2 to n-1
    for (int i = 2; i < n; i++)
        if (n % i == 0)
            return false;

    return true;
}
int main(int argc, char *argv[])
{
    if (argc<4) {
        printf("Usage: ./2prob producers buffer_size workers\n");
        return 0;
    }
    P = atoi(argv[1]);
    BUF_SIZE = atoi(argv[2]);
    W = atoi(argv[3]);

    buffer = (int* )malloc(BUF_SIZE*(sizeof(int)));
    NUM_THREADS = P+W;
    int thread_args[NUM_THREADS], retval;
    pthread_t threads[NUM_THREADS]; /* thread identifiers */
    int i;
    is.open("input.txt");
    os.open("prime.txt");

    /* create the threads; may be any number, in general */
    for (int i = 0; i < NUM_THREADS; i++)
    {
        if (i % 2 == 0)
        {
            thread_args[i] = i;
            retval = pthread_create(&threads[i], NULL, producer, (void *)&thread_args[i]);
        }
        else
        {
            thread_args[i] = i;
            retval = pthread_create(&threads[i], NULL, consumer, (void *)&thread_args[i]);
        }
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        // second argument is a buffer for return value or 0.
        if (i % 2 == 0)
            retval = pthread_join(threads[i], 0);
    }
    printf("========== producers completed ========== %d\n", done);

    // waiting for consumers to finish
    while (num != 0)
    {
        ;
    }

    flag = 1;
    // cancelling consumers since they will never terminate
    // also verified that consumers have finished reading.
    for (int i = 0; i < NUM_THREADS; i++)
    {
        // second argument is a buffer for return value or 0.
        if (i % 2 != 0)
            retval = pthread_cancel(threads[i]);
    }
    printf("========== consumers completed ==========\n");
    //std::cout<<nos.size()<<std::endl;
    //std::cout<<distinct_elements(nos)<<std::endl;
    //printf("%d %d\n", totprod, totcon);
    return 0;
}

void *producer(void *param)
{
    int i, number;
    int id = *((int *)param);
    while (1)
    {
        pthread_mutex_lock(&m);
        if (is.eof()) {
            pthread_mutex_unlock(&m);
            return 0;
        }
        std::string line;
        std::getline(is, line);
        if (!line.empty()){
            
            number = stoi(line);
        }
        else
        {
            // this means EOF has been reached. i
            pthread_mutex_unlock(&m);
            continue;
            
        }
        if (num > BUF_SIZE)
            exit(1);

        while (num == BUF_SIZE)
        {
            pthread_cond_wait(&c_prod, &m);
        }
        buffer[add] = number;
        nos.push_back(number);
        add = (add + 1) % BUF_SIZE;
        num++;
        
        pthread_mutex_unlock(&m);
        pthread_cond_signal(&c_cons);
    }
   
}

void *consumer(void *param)
{
    int i;
    int id = *((int *)param);
    while (1)
    {
        pthread_mutex_lock(&m);
        if (num < 0)
            exit(1);
        while (num == 0)
        {
            pthread_cond_wait(&c_cons, &m);
        }

        i = buffer[rem];
        rem = (rem + 1) % BUF_SIZE;
        num--;
        totcon++;
        if (isPrime(i))
        {
            os<<i<<std::endl;
        }
        pthread_mutex_unlock(&m);
        pthread_cond_signal(&c_prod);
    }
}

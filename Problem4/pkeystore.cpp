/*Job Queue, manager, worker thread creation and responsibilities as mentioned in assignment are implemented here*/
#include "pkeystore.h"
#include <stdio.h>

#define QSIZE 8

int init_flag = 0; // will be set to 1 after the first enqueue call

int status_arr[QSIZE];
struct operation *queue[QSIZE];
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;       // mutex lock for job queue
pthread_mutex_t h_table = PTHREAD_MUTEX_INITIALIZER; // mutex lock for hash table

pthread_cond_t c_cons = PTHREAD_COND_INITIALIZER; /* consumer waits on this cond var */
pthread_cond_t c_prod = PTHREAD_COND_INITIALIZER; /* producer waits on this cond var */
int num = 0;                                      // number of elements in job queue
int rem = 0, add = 0;                             // keep track of index of job queue
std::unordered_map<int, int> map;

void *perform_operation(void *param)
{
    // the function that will be run by  each thread

    while (1)
    {
        struct operation *op;

        // get lock on queue
        pthread_mutex_lock(&m);
        if (num < 0)
            exit(1);
        while (num == 0)
        {
            pthread_cond_wait(&c_cons, &m);
        }
        op = queue[rem];
        rem = (rem + 1) % QSIZE;
        num--;
        // release lock on queue
        pthread_mutex_unlock(&m);

        // acquire lock on hash table
        pthread_mutex_lock(&h_table);
        // check the operation type and perform the operation
        if (op->type == 0)
        {
            // insert
            map[op->key] = op->value;
        }
        else if (op->type == 1)
        {
            // update
            if (map.find(op->key) == map.end())
            {
                // not clear what to do if element not found
                ;
            }
            else
            {
                map[op->key] = op->value;
            }
        }
        else if (op->type == 2)
        {
            // delete
            if (map.find(op->key) == map.end())
            {
                // not clear what to do if element not found
                ;
            }
            else
            {
                map.erase(op->key);
            }
        }
        else
        {
        }
        // release lock on hash table
        pthread_mutex_unlock(&h_table);
        pthread_cond_signal(&c_prod);
    }
}

int init_threads()
{
    init_flag = 1;
    pthread_t threads[4];

    for (int i = 0; i < 4; i++)
    {

        int retval = pthread_create(&threads[i], NULL, perform_operation, NULL);
    }
}

int enqueue(struct operation *op)
{
    int jobloc;
    printf("called enqueue\n");
    if (init_flag == 0)
    {
         // initialize on first call to enqueue
        init_threads();
    }

    // acquire lock on queue
    pthread_mutex_lock(&m);

    if (num > QSIZE)
        exit(1);

    if (num == QSIZE)
    {
        // queue is full, returning -1 since non blocking
        std::cout << "job queue full, returning\n";
        jobloc = -1;
    }
    else
    {
        // add to job queue
        jobloc = add;
        queue[add] = op;

        add = (add + 1) % QSIZE;
        num++;
    }
    // release lock on queue
    pthread_mutex_unlock(&m);
    pthread_cond_signal(&c_cons);

    return jobloc;
}



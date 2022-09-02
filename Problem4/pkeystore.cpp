/*Job Queue, manager, worker thread creation and responsibilities as mentioned in assignment are implemented here*/
#include "pkeystore.h"
#include <stdio.h>
#include <pthread.h>
#include <unordered_map>

#define QSIZE 8

int init_flag = 0; // will be set to 1 after the first enqueue call

int status_arr[QSIZE];
struct operation *queue[QSIZE];
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;       // mutex lock for job queue
pthread_mutex_t h_table = PTHREAD_MUTEX_INITIALIZER; // mutex lock for hash table
pthread_mutex_t st = PTHREAD_MUTEX_INITIALIZER;      // mutex for status array
pthread_cond_t c_cons = PTHREAD_COND_INITIALIZER;    /* consumer waits on this cond var */
pthread_cond_t c_prod = PTHREAD_COND_INITIALIZER;    /* producer waits on this cond var */
int num = 0;                                         // number of elements in job queue
int rem = 0, add = 0;                                // keep track of index of job queue
std::unordered_map<int, int> map;

int perform_operation()
{
    // the function that will be run by  each thread

    while (1)
    {
        struct operation *op;
        int status;
        // get lock on hash table
        pthread_mutex_lock(&h_table);
        if (num < 0)
            exit(1);
        while (num == 0)
        {
            pthread_cond_wait(&c_cons, &m);
        }
        int jobloc = rem;
        op = queue[rem];
        rem = (rem + 1) % QSIZE;
        num--;
        // check the operation type and perform the operation
        if (op->type == 0)
        {
            // insert
            map[op->key] = op->value;
            pthread_mutex_lock(&st);
            status_arr[rem] = 1;
            pthread_mutex_unlock(&st);
        }
        else if (op->type == 1)
        {
            // update
            if (map.find(op->key) == map.end())
                jobloc = 0; // element not found
            else
            {
                map[op->key] = op->value;
            }
            // updating status array
            pthread_mutex_lock(&st);
            status_arr[rem] = 1;
            pthread_mutex_unlock(&st);
        }
        else
        {
            // delete
            if (map.find(op->key) == map.end())
                status = 0; // element not found
            else
            {
                map.erase(op->key);
                status = 1;
            }
        }
        // updating status array
        pthread_mutex_lock(&st);
        status_arr[rem] = status;
        pthread_mutex_unlock(&st);
        // release lock on hash table
        pthread_mutex_unlock(&h_table);
        pthread_cond_signal(&c_prod);
    }
}

int enqueue(struct operation *op)
{
    int jobloc;
    printf("called enqueue\n");
    if (init_flag == 0)
    {
        // initialize
        init_flag = 1;
        pthread_t threads[4];

        for (int i = 0; i < 4; i++)
        {

            int retval = pthread_create(&threads[i], NULL, perform_operation, NULL);
        }
        pthread_mutex_lock(&m);

        if (num > QSIZE)
            exit(1);

        while (num == QSIZE)
        {
            pthread_cond_wait(&c_prod, &m);
        }

        // add to job queue
        jobloc = add;
        queue[add] = op;

        add = (add + 1) % QSIZE;
        num++;

        pthread_mutex_unlock(&m);
        pthread_cond_signal(&c_cons);
    }
    else
    {

        pthread_mutex_lock(&m);

        if (num > QSIZE)
            exit(1);

        while (num == QSIZE)
        {
            pthread_cond_wait(&c_prod, &m);
        }

        // add to job queue
        jobloc = add;
        queue[add] = op;

        add = (add + 1) % QSIZE;
        num++;

        pthread_mutex_unlock(&m);
        pthread_cond_signal(&c_cons);
    }

    return jobloc;
}

int check_status(int jobloc)
{
    printf("called check status\n");
    return 1;
}

uint64_t find(uint32_t key)
{
    printf("called find\n");
    int jobloc;
    pthread_mutex_lock(&h_table);
    if (map.find(key) == map.end())
    {
        jobloc = 0;
        pthread_mutex_unlock(&h_table);
        return -1;
    }
    else
    {
        jobloc = 1;
        int value = map[key];
        pthread_mutex_unlock(&h_table);
        return value;
    }
}

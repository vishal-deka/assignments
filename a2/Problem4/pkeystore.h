#ifndef __PKEY__FILE_H
#define __PKEY__FILE_H

#include <stdint.h>
#include <iostream>
#include <pthread.h>
#include <unordered_map>

struct operation{
    uint8_t type;
    uint32_t key;
    uint64_t value;
};

int enqueue(struct operation *op);


#endif

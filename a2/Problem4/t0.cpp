#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "./pkeystore.h"

#define NUM_ELEMS 50
#define MAX_KEY (1 << 10)
#define MAX_VALUE (1 << 20)
#define NUM_OPERS 100
#define INVALID (-2)

struct jobdetails
{
    int valid; // = 1 means the job status is not yet checked
    int jobloc;
    struct operation *op;
};
void calculate(uint64_t value)
{
    int i = 0;
    while (value >> 1)
    {
        i += 1;
        value = value >> 1;
    }
    printf("i: %d\n", i);
}

int main(int argc, char *argv[])
{
    int numops = atoi(argv[1]);
    struct operation ops[numops];
    int pos = 0;
    struct jobdetails jd[4];
    int status[numops];
    uint64_t val;
    int j = 0;
    std::cout<<"inserting\n";
    for (int i = 0; i < 5; i++)
    {
        ops[i].type = 0;
        ops[i].key = i;
        ops[i].value = 99+i;
        std::cout<<enqueue(&ops[i])<<"\n";
    }
    std::cout<<"del\n";
    for (int i = 0; i < 4; i++){
        ops[5+i].type = 2;
        ops[5+i].key = i;
        std::cout<<enqueue(&ops[5+i])<<"\n";
    }
    std::cout<<"find\n";
    for (int i = 0; i < 5; i++)
    {
        ops[i].type = 3;
        ops[i].key = i;
        //ops[i].value = 99+i;
        std::cout<<enqueue(&ops[i])<<"\n";
    }
   
    std::cout<<"\n";
    return 0;
}

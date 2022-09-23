// g++ -O2 -fopenmp -o problem4 problem4.cpp

#include <cassert>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <omp.h>

using std::cout;
using std::endl;

#define N (1 << 24)

// Number of array elements a task will process
#define GRANULARITY (1 << 10)

uint64_t reference_sum(uint32_t *A)
{
  uint64_t seq_sum = 0;
  for (int i = 0; i < N; i++)
  {
    seq_sum += A[i];
  }
  return seq_sum;
}

uint64_t par_sum_omp_nored(uint32_t *A)
{
  // SB: Write your OpenMP code here
  uint64_t globalsum = 0;
  #pragma omp parallel
  {
    int id = omp_get_thread_num();
    uint64_t local = 0;
    #pragma omp for
    for(int i=0; i<N;i++) {
      local += A[i];
    }

    #pragma omp atomic
    globalsum+=local;
  }
  return globalsum;
}

uint64_t par_sum_omp_red(uint32_t *A)
{
  uint64_t sum =0;
// SB: Write your OpenMP code here
#pragma omp parallel for reduction(+: sum)
  for (int i = 0; i < N; i++)
    sum = sum + A[i];
  return sum;
}
uint64_t gsum = 0;
void partialSum(uint32_t *A, int offset, int chunksize){
  uint64_t local=0;
  for (int i=offset; i<offset+chunksize; i++){
    local+=A[i];
  }
  #pragma omp atomic
  gsum += local;
}

uint64_t par_sum_omp_tasks(uint32_t *A)
{
  // SB: Write your OpenMP code here
  uint64_t sum=0;
  #pragma omp parallel num_threads(8)
  {
    uint64_t loc = 0;
    #pragma omp single
    {
      for(int i=0;i<omp_get_num_threads();i++){
        int csize = (int)N/omp_get_num_threads();
        if(i==7) csize+= N%omp_get_num_threads();
        int ofst = i*csize;
        #pragma omp task
        partialSum(A,ofst , csize);
      }
    }
  }
  return gsum;
}

int main()
{
  uint32_t *x = new uint32_t[N];
  for (int i = 0; i < N; i++)
  {
    x[i] = i;
  }

  double start_time, end_time, pi;

  start_time = omp_get_wtime();
  uint64_t seq_sum = reference_sum(x);
  end_time = omp_get_wtime();
  cout << "Sequential sum: " << seq_sum << " in " << (end_time - start_time) << " seconds\n";

  start_time = omp_get_wtime();
  uint64_t par_sum = par_sum_omp_nored(x);
  end_time = omp_get_wtime();
  assert(seq_sum == par_sum);
  cout << "Parallel sum (thread-local, atomic): " << par_sum << " in " << (end_time - start_time)
       << " seconds\n";

  start_time = omp_get_wtime();
  uint64_t ws_sum = par_sum_omp_red(x);
  end_time = omp_get_wtime();
  assert(seq_sum == ws_sum);
  cout << "Parallel sum (worksharing construct): " << ws_sum << " in " << (end_time - start_time)
       << " seconds\n";

  start_time = omp_get_wtime();
  uint64_t task_sum = par_sum_omp_tasks(x);
  end_time = omp_get_wtime();
  if (seq_sum != task_sum)
  {
    cout << "Seq sum: " << seq_sum << " Task sum: " << task_sum << "\n";
  }
  assert(seq_sum == task_sum);
  cout << "Parallel sum (OpenMP tasks): " << task_sum << " in " << (end_time - start_time)
       << " seconds\n";

  return EXIT_SUCCESS;
}

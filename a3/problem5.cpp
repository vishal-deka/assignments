// g++ -msse4 -mavx -march=native -O3 -fopt-info-vec-optimized -fopt-info-vec-missed -fopenmp -o problem5 problem5.cpp

#include <cassert>
#include <chrono>
#include <climits>
#include <cstdlib>
#include <emmintrin.h>
#include <immintrin.h>
#include <iostream>
#include <omp.h>
#include<stdio.h>
typedef __m256i v8i;
typedef __m128i v4i;

using std::cout;
using std::endl;
using std::chrono::duration_cast;
using HR = std::chrono::high_resolution_clock;
using HRTimer = HR::time_point;
using std::chrono::microseconds;

#define N (1 << 16)
#define SSE_WIDTH 128
#define AVX2_WIDTH 256
#define NUM_THREADS 8

void print_array(int* array);

__attribute__((optimize("no-tree-vectorize"))) int ref_version(int* __restrict__ source,
                                                               int* __restrict__ dest) {
  __builtin_assume_aligned(source, 64);
  __builtin_assume_aligned(dest, 64);

  int tmp = 0;
  for (int i = 0; i < N; i++) {
    tmp += source[i];
    dest[i] = tmp;
  }
  return tmp;
}

int omp_version(int* source, int* dest) { 
  int chunksize = N/NUM_THREADS;
  //temporary array to hold the partial sums
  int t_sums[NUM_THREADS+1];
  t_sums[0]=0;  // 0 in first element since first chunk of dest will not be added to anything
  // computing partial sums in parallel
  #pragma omp parallel num_threads(NUM_THREADS)
  {
    int id = omp_get_thread_num();
    int loc =0;
    #pragma omp for schedule(static,  chunksize)
    for(int i=0;i<N;i++){
      loc+=source[i];
      dest[i] = loc;
    }
    t_sums[id+1] = dest[id*chunksize+chunksize-1];
  }
  for(int i=2;i<NUM_THREADS;i++){
    t_sums[i] += t_sums[i-1];
  }
  //adding the partial sums back to the original array
  #pragma omp parallel num_threads(NUM_THREADS)
  {
    int id = omp_get_thread_num();
    #pragma omp for schedule(static, chunksize)
    for(int i=0;i<N;i++){
      dest[i] += t_sums[id];
    }
  }

  return dest[N-1];
}


void prefix(int *p, int* q) {
    v8i x = _mm256_load_si256((v8i*) p);
    x = _mm256_add_epi32(x, _mm256_slli_si256(x, 4));
    x = _mm256_add_epi32(x, _mm256_slli_si256(x, 8));
    _mm256_store_si256((v8i*) q, x);
}
void prefix_sse(int *p, int* q) {
    v4i x = _mm_load_si128((v4i*) p);
    x = _mm_add_epi32(x, _mm_slli_si128(x, 4));
    x = _mm_add_epi32(x, _mm_slli_si128(x, 8));
    _mm_store_si128((v4i*) q, x);
}


v4i accumulate(int *p, v4i s) {
    v4i d = (v4i) _mm_broadcast_ss((float*) &p[3]);
    v4i x = _mm_load_si128((v4i*) p);
    x = _mm_add_epi32(s, x);
    _mm_store_si128((v4i*) p, x);
    return _mm_add_epi32(s, d);
}

int sse4_version(int* source, int* dest) { 
  for (int i = 0; i < N; i += 4)
        prefix_sse(&source[i], &dest[i]);
    
    v4i s = _mm_setzero_si128();
    
    for (int i = 0; i < N; i += 4)
        s = accumulate(&dest[i], s);

    return dest[N-1];
}


int avx2_version(int* source, int* dest) { 
  for (int i = 0; i < N; i += 8)
        prefix(&source[i], &dest[i]);
    
    v4i s = _mm_setzero_si128();
    
    for (int i = 0; i < N; i += 4)
        s = accumulate(&dest[i], s);
    return dest[N-1];
 }

int* array = nullptr;
int* ref_res = nullptr;
int* omp_res = nullptr;
int* sse_res = nullptr;
int* avx2_res = nullptr;

__attribute__((optimize("no-tree-vectorize"))) int main() {
  array = static_cast<int*>(aligned_alloc(64, N * sizeof(int)));
  ref_res = static_cast<int*>(aligned_alloc(64, N * sizeof(int)));
  omp_res = static_cast<int*>(aligned_alloc(64, N * sizeof(int)));
  sse_res = static_cast<int*>(aligned_alloc(64, N * sizeof(int)));
  avx2_res = static_cast<int*>(aligned_alloc(64, N * sizeof(int)));

  for (int i = 0; i < N; i++) {
    array[i] = 1;
    ref_res[i] = 0;
    omp_res[i] = 0;
    sse_res[i] = 0;
    avx2_res[i] = 0;
  }

  
  HRTimer start = HR::now();
  int val_ser = ref_version(array, ref_res);
  HRTimer end = HR::now();
  auto duration = duration_cast<microseconds>(end - start).count();
  cout << "Serial version: " << val_ser << " time: " << duration << endl;

  start = HR::now();
  int val_omp = omp_version(array, omp_res);
  end = HR::now();
  duration = duration_cast<microseconds>(end - start).count();
  assert(val_ser == val_omp || printf("OMP result is wrong!\n"));
  cout << "OMP version: " << val_omp << " time: " << duration << endl;
  
  start = HR::now();
  int val_sse = sse4_version(array, sse_res);
  end = HR::now();
  duration = duration_cast<microseconds>(end - start).count();
  assert(val_ser == val_sse || printf("SSE4 result is wrong!\n"));
  cout << "SSE4 version: " << val_sse << " time: " << duration << endl;

  start = HR::now();
  int val_avx = avx2_version(array, avx2_res);
  end = HR::now();
  duration = duration_cast<microseconds>(end - start).count();
  assert(val_ser == val_avx || printf("AVX2 result is wrong!\n"));
  cout << "AVX2 version: " << val_avx << " time: " << duration << endl;

  return EXIT_SUCCESS;
}

void print_array(int* array) {
  for (int i = 0; i < N; i++) {
    cout << array[i] << "\t";
  }
  cout << "\n";
}

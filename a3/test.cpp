#include<immintrin.h>
#include<stdio.h>
#include<iostream>

#include<cmath>
typedef __m256i v8i;
typedef __m128i v4i;

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
void getprefix(int *a, int* b, int n) {
    for (int i = 0; i < n; i += 8)
        prefix(&a[i], &b[i]);
    
    v4i s = _mm_setzero_si128();
    
    for (int i = 0; i < n; i += 4)
        s = accumulate(&b[i], s);
}
void getprefix_sse(int *a, int *b,int n) {
    for (int i = 0; i < n; i += 4)
        prefix_sse(&a[i], &b[i]);
    
    v4i s = _mm_setzero_si128();
    
    for (int i = 0; i < n; i += 4)
        s = accumulate(&b[i] ,s);
}


int main(){
   int N = 1024;
   

   int* A = (int* )_mm_malloc(N*sizeof(int),32);
   int* B = (int* )_mm_malloc(N*sizeof(int),32);
   for(int i=0; i<N; i++) A[i] =1;
   getprefix_sse(A,B,N);
   std::cout<<B[1023]<<"\n";
}
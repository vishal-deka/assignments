// Compile: g++ -O2 -mavx -march=native -o problem2 problem2.cpp
// Execute: ./problem2

#include <cmath>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include<immintrin.h>

using std::cout;
using std::endl;
using std::ios;

const int N = (1 << 13);
const int Niter = 10;
const double THRESHOLD = 0.000001;

double rtclock()
{
  struct timezone Tzp;
  struct timeval Tp;
  int stat;
  stat = gettimeofday(&Tp, &Tzp);
  if (stat != 0)
  {
    cout << "Error return from gettimeofday: " << stat << endl;
  }
  return (Tp.tv_sec + Tp.tv_usec * 1.0e-6);
}

void reference(double **A, double *x, double *y_ref, double *z_ref)
{
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      y_ref[j] = y_ref[j] + A[i][j] * x[i];
      z_ref[j] = z_ref[j] + A[j][i] * x[i];
    }
  }
}

void check_result(double *w_ref, double *w_opt)
{
  double maxdiff = 0.0, this_diff = 0.0;
  int numdiffs = 0;

  for (int i = 0; i < N; i++)
  {
    this_diff = w_ref[i] - w_opt[i];
    if (fabs(this_diff) > THRESHOLD)
    {
      numdiffs++;
      if (this_diff > maxdiff)
        maxdiff = this_diff;
    }
  }

  if (numdiffs > 0)
  {
    cout << numdiffs << " Diffs found over threshold " << THRESHOLD << "; Max Diff = " << maxdiff
         << endl;
  }
  else
  {
    cout << "No differences found between base and test versions\n";
  }
}

// TODO: INITIALLY IDENTICAL TO REFERENCE; MAKE YOUR CHANGES TO OPTIMIZE THE CODE
// You can create multiple versions of the optimized() function to test your changes
void optimized(double **__restrict__ A, double *__restrict__ x, double *__restrict__ y_opt, double *__restrict__ z_opt)
{
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      y_opt[j] = y_opt[j] + A[i][j] * x[i];
    }
  }
  for (int j = 0; j < N; j++)
  {
    for (int i = 0; i < N; i++)
    {
      z_opt[j] = z_opt[j] + A[j][i] * x[i];
    }
  }
}

// void optimized(double** A, double* x, double* y_opt, double* z_opt) {
//   #pragma gcc ivdep
//   for (int i = 0; i < N; i++) {
//     for (int j = 0; j < N; j++) {
//       y_opt[j] = y_opt[j] + A[i][j] * x[i];
//       z_opt[j] = z_opt[j] + A[j][i] * x[i];
//     }
//   }
// }

void avx_version(double **A, double *x, double *y_opt, double *z_opt) {
  __m256d a,b,c,temp;
  for (int i = 0; i < N; i++)
  {
    c = _mm256_broadcast_sd(&x[i]);
    for (int j = 0; j < N; j+=4)
    {
      //y_opt[j] = y_opt[j] + A[i][j] * x[i];
      a = _mm256_load_pd(&y_opt[j]);
      b = _mm256_load_pd(&A[i][j]);
      
      temp = _mm256_mul_pd(b,c);
      a = _mm256_add_pd(a,temp);
      _mm256_store_pd(&y_opt[j],a);

    }
  }
  double * buf =(double* )_mm_malloc(4*sizeof(double),32);;
  for (int j = 0; j < N; j++)
  {
    c = _mm256_broadcast_sd(&z_opt[j]);
    for (int i = 0; i < N; i+=4)
    {
      //z_opt[j] = z_opt[j] + A[j][i] * x[i];
      a = _mm256_load_pd(&x[i]);
      b = _mm256_load_pd(&A[j][i]);
      
      temp = _mm256_mul_pd(a,b);
      c = _mm256_add_pd(c,temp);
      
    }
    _mm256_store_pd(&buf[0],c);
    z_opt[j] = buf[0];

  }
}

int main()
{
  double clkbegin, clkend;
  double t;

  int i, j, it;
  cout.setf(ios::fixed, ios::floatfield);
  cout.precision(5);

  double **A;
  A = new double *[N];
  for (int i = 0; i < N; i++)
  {
    A[i] = (double* )_mm_malloc(N*sizeof(double),32);
  }

  double *x, *y_ref, *z_ref, *y_opt, *z_opt;
  x = new double[N];
  x = (double* )_mm_malloc(N*sizeof(double),32);
  y_ref = new double[N];
  z_ref = new double[N];
  //y_opt = new double[N];
  y_opt = (double* )_mm_malloc(N*sizeof(double),32);
  //z_opt = new double[N];
  z_opt = (double* )_mm_malloc(N*sizeof(double),32);
  for (i = 0; i < N; i++)
  {
    x[i] = i;
    y_ref[i] = 1.0;
    y_opt[i] = 1.0;
    z_ref[i] = 2.0;
    z_opt[i] = 2.0;
    for (j = 0; j < N; j++)
    {
      A[i][j] = (i + 2.0 * j) / (2.0 * N);
    }
  }

  clkbegin = rtclock();
  for (it = 0; it < Niter; it++)
  {
    reference(A, x, y_ref, z_ref);
  }
  clkend = rtclock();
  t = clkend - clkbegin;
  cout << "Reference Version: Matrix Size = " << N << ", " << 4.0 * 1e-9 * N * N * Niter / t
       << " GFLOPS; Time = " << t / Niter << " sec\n";

  clkbegin = rtclock();
  for (it = 0; it < Niter; it++)
  {
    optimized(A, x, y_opt, z_opt);
  }
  clkend = rtclock();
  t = clkend - clkbegin;
  cout << "Optimized Version: Matrix Size = " << N << ", Time = " << t / Niter << " sec\n";
  check_result(y_ref, y_opt);

  // Reset
  for (i = 0; i < N; i++)
  {
    y_opt[i] = 1.0;
    z_opt[i] = 2.0;
  }

  // Another optimized version possibly

  // Version with intinsics

  clkbegin = rtclock();
  for (it = 0; it < Niter; it++)
  {
    avx_version(A, x, y_opt, z_opt);
  }
  clkend = rtclock();
  t = clkend - clkbegin;
  cout << "Intrinsics Version: Matrix Size = " << N << ", Time = " << t / Niter << " sec\n";
  check_result(y_ref, y_opt);

  return EXIT_SUCCESS;
}

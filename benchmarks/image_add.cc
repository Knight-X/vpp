#include <iostream>
#include <fstream>
#include <vpp/boxNd.hh>
#include <vpp/vpp.hh>

#include "get_time.hh"

using namespace vpp;


void raw_naive(image2d<int> A, image2d<int> B, image2d<int> C)
{
  for (int r = 0; r < A.nrows(); r++)
  for (int c = 0; c < A.ncols(); c++)
  {
    A(r, c) = B(r, c) + C(r, c);
    // equivalent to pa[r * A.ncols() + c] = pb[r * B.ncols() + c] + pc[r * C.ncols() + c];

  }
}

void check(image2d<int> A, image2d<int> B, image2d<int> C)
{
  for (int r = 0; r < A.nrows(); r++)
  for (int c = 0; c < A.ncols(); c++)
  {
    assert(A(r, c) == B(r, c) + C(r, c));
  }
}


void raw_openmp_simd(image2d<int> A, image2d<int> B, image2d<int> C)
{
  int nr = A.nrows();
#pragma omp parallel for
  for (int r = 0; r < nr; r++)
  {
    int* curA = &A(vint2(r, 0));
    int* curB = &B(vint2(r, 0));
    int* curC = &C(vint2(r, 0));
    int* endA = curA + A.nrows();

    int nc = A.ncols();
#pragma omp simd
    for (int i = 0; i < nc; i++)
    {
      curA[i] = curB[i] + curC[i];
    }
  }
}

void vpp_pixel_wise(image2d<int> A, image2d<int> B, image2d<int> C)
{
  vpp::pixel_wise(A, B, C) << [] (int& a, int& b, int& c)
  {
    a = b + c;
  };
}

template <typename T>
int bench(int size, T& results)
{
  std::cout << size << std::endl;
  image2d<int> A(sqrt(size),sqrt(size));
  image2d<int> B(A.domain());
  image2d<int> C(A.domain());

  fill(A, 0);

  pixel_wise(B, C) << [] (int& b, int& c) { b = rand(); c = rand(); };

  int K = 400;
  double time;


  // Cache warm up.
  raw_naive(A, B, C);

  fill(A, 0);
  time = get_time_in_seconds();
  for (int k = 0; k < K; k++)
    raw_naive(A, B, C);
  double raw_naive_time = get_time_in_seconds() - time;
  check(A, B, C);

  fill(A, 0);
  time = get_time_in_seconds();
  for (int k = 0; k < K; k++)
    raw_openmp_simd(A, B, C);
  double raw_openmp_simd_time = get_time_in_seconds() - time;
  check(A, B, C);

  fill(A, 0);
  time = get_time_in_seconds();
  for (int k = 0; k < K; k++)
    vpp_pixel_wise(A, B, C);
  double pixel_wise_time = get_time_in_seconds() - time;
  check(A, B, C);

  // std::cout << "time per iteration (ms) : " << std::endl;
  // std::cout << "raw_naive_time: " << 1000. * raw_naive_time / K << std::endl;
  // std::cout << "raw_naive2_time: " << 1000. * raw_naive2_time / K << std::endl;
  // std::cout << "raw_sequential_time: " << 1000. * raw_sequential_time / K << std::endl;
  // std::cout << "raw_openmp_time: " << 1000. * raw_openmp_time / K << std::endl;
  // std::cout << "raw_openmp_simd_time: " << 1000. * raw_openmp_simd_time / K << std::endl;
  // std::cout << "vpp_pixel_wise: " << 1000. * pixel_wise_time / K << std::endl;
  //std::cout << "domain iteration overhead: " << 100. * id_time / ref_time - 100. << "%" << std::endl;

  double freq = 3.7 * 1000 * 1000 * 1000;

  results.naive.push_back(freq * raw_naive_time / (K * size));
  results.pixel_wise.push_back(freq * pixel_wise_time / (K * size));
  results.raw.push_back(freq * raw_openmp_simd_time / (K * size));

  // results.naive.push_back(1000 * raw_naive_time / K);
  // results.pixel_wise.push_back(1000 * pixel_wise_time / K);
  // results.raw.push_back(1000 * raw_openmp_simd_time / K);
}

int main()
{

  ::sched_param sc_params;
  sc_params.sched_priority = 1;
  if (::sched_setscheduler(::getpid(), SCHED_FIFO, &sc_params))
    ::fprintf(stderr, "sched_setscheduler(): %s\n", ::strerror(errno));

  struct {
    std::vector<float> naive;
    std::vector<float> pixel_wise;
    std::vector<float> raw;
  } results;

  int step = 10000;
  int max_size = 1000000;
  for (int s = 0; s < max_size; s += step)
    bench(s, results);

  std::ofstream n("add_naive.txt");
  std::ofstream p("add_pixel_wise.txt");
  std::ofstream o("add_openmp.txt");
  std::ofstream speedup("add_speedup.txt");
  for (int s = 0; s < max_size/step; s ++)
  {
    n << s * step << '\t'<< results.naive[s] << std::endl;
    p << s * step << '\t'<< results.pixel_wise[s] << std::endl;
    o << s * step << '\t'<< results.raw[s] << std::endl;
    speedup << s * step << '\t'<< results.naive[s] / results.pixel_wise[s] << std::endl;
  }
}

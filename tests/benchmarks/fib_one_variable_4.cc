/* 
 * Adapted from: https://svn.sosy-lab.org/software/sv-benchmarks/trunk/c/pthread/fib_bench_true-unreach-call.c 
 * Written by Magnus Lang
*/

#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

#define IJ(I, J) ((uint64_t)(I) << 32 | (J))
#define I(ij) ((uint32_t)((ij) >> 32))
#define J(ij) ((uint32_t)((ij) & (((uint64_t)2<<32)-1)))

atomic<int> ij;

#define N 4

void* t1(void* arg) {
    for (int k = 0; k < N; k++){
      // i+=j;
      uint64_t tmpj = atomic_load_explicit(&ij, memory_order_acquire);
      uint64_t tmpi = atomic_load_explicit(&ij, memory_order_acquire);
      ij.store(IJ(I(tmpi)+J(tmpj), J(tmpj)), memory_order_release);
	  cout << "t1 " << tmpj << " " << tmpi << " " << (IJ(I(tmpi)+J(tmpj), J(tmpj))) << endl;
    }
	return NULL;
}

void* t2(void* arg) {
    for (int k = 0; k < N; k++) {
      // j+=i;
      uint64_t tmpi = atomic_load_explicit(&ij, memory_order_acquire);
      uint64_t tmpj = atomic_load_explicit(&ij, memory_order_acquire);
      ij.store(IJ(I(tmpi), I(tmpi)+J(tmpj)), memory_order_release);
	  cout << "t2 " << tmpj << " " << tmpi << " " << (IJ(I(tmpi), I(tmpi)+J(tmpj))) << endl;
    }
	return NULL;
  }

static int fib(int n) {
    int cur = 1, prev = 0;
    while(n--) {
      int next = prev+cur;
      prev = cur;
      cur = next;
    }
    return prev;
}

int main() {
    pthread_t id1, id2;
  
    ij.store(IJ(1, 1));
  
    pthread_create(&id1, NULL, t1, NULL);
    pthread_create(&id2, NULL, t2, NULL);

    int correct = fib(2+2*N);
    uint64_t tmpi = atomic_load_explicit(&ij, memory_order_acquire);
    uint64_t tmpj = atomic_load_explicit(&ij, memory_order_acquire);
	cout << "main "<< I(tmpi) << " " << J(tmpj) << endl;
    if (I(tmpi) > correct || J(tmpj) > correct) {
      assert(0);
    }

    return 0;
}
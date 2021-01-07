#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

#define REL memory_order_release
#define ACQ memory_order_acquire
#define ACQREL memory_order_acq_rel

extern void assume(bool);
extern void nondet_int(atomic<int> &x, int start_range, int end_range);
extern void nondet_int(int &x, int start_range, int end_range);

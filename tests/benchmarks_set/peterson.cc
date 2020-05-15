/* Copyright (C) 2018 Tuan Phong Ngo 
 * This benchmark is part of TRACER
 */

/*
 * Peterson's critical section algorithm, implemented with fences.
 *
 * Adapted from: https://svn.sosy-lab.org/software/sv-benchmarks/trunk/c/pthread-atomic/peterson_true-unreach-call.c
 */

#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;


// unfolding is done as per following parameters
#define LOOP 2
#define OUTER_LOOP 3 
#define LB LOOP*OUTER_LOOP

atomic<int> flag0;
atomic<int> flag1;
atomic<int> turn;

atomic_int var; // to avoid race

atomic_int __fence_var;

void *p0(void *arg)
{
  for (int j=0; j<OUTER_LOOP; j++) {
    flag0.store(1, memory_order_release);
    __fence_var.fetch_add(0, memory_order_acq_rel);

    turn.store(1, memory_order_release);
    __fence_var.fetch_add(0, memory_order_acq_rel);

    int ok = 0;
    
    for (int i=0; i<LOOP; i++) { 
      int _flag1 = flag1.load(memory_order_acquire);
      int _turn =  turn.load(memory_order_acquire);
      if ( _flag1 != 1 || _turn != 1) {
        ok = 1;	
        break;
      } 
    }
    if (ok==0) return NULL;

    // critical section
    var.store(1, memory_order_release);
	int tmp = var.load(memory_order_acquire);
    assert(tmp==1);

    flag0.store(0, memory_order_release);
  }
  return NULL;
}

void* p1(void *arg)
{
  for (int j=0; j<OUTER_LOOP; j++) {
    flag1.store(1, memory_order_release);
    __fence_var.fetch_add(0, memory_order_acq_rel);

    atomic_store_explicit(&turn, 0, memory_order_release);
    __fence_var.fetch_add(0, memory_order_acq_rel);

    int ok = 0;
    for (int i=0; i<LOOP; i++) {
      int _flag0 = flag0.load(memory_order_acquire);
      int _turn =  turn.load(memory_order_acquire);
      if (_flag0 != 1 || _turn != 0) {
        ok = 1;
        break;		
      } 
    }
    if (ok==0) return NULL; 

    // critical section
    var.store(2, memory_order_release);
	int tmp = var.load(memory_order_acquire);
    assert(tmp==2);

    flag1.store(0, memory_order_release);
  }
  return NULL;
}

int main()
{
 pthread_t a, b;

 pthread_create(&a, NULL, p0, NULL);
 pthread_create(&b, NULL, p1, NULL);

 pthread_join(a, NULL);
 pthread_join(b, NULL);

 return 0;
}
/* Copyright (C) 2018 Tuan Phong Ngo 
 * This benchmark is part of TRACER
 */

/*
 * Szymaski's critical section algorithm, implemented with fences.
 *
 * Adapted from: https://svn.sosy-lab.org/software/sv-benchmarks/trunk/c/pthread-atomic/szymanski_true-unreach-call.c
 */


#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

#define LOOP 3
#define OUTER_LOOP 2
#define LB LOOP*OUTER_LOOP

// shared variables
atomic_int flag1; 
atomic_int flag2;

atomic_int var;

atomic_int __fence_var;

void *p1(void *arg)
{
  int ok;
  // for (int j=0; j<OUTER_LOOP; j++) {    // j loop: begin
    flag1.store(1, memory_order_release);

    ok = 0;
    // for (int i=0; i<LOOP; i++) {        // i loop: begin
    int tmp1=flag2.load(memory_order_acquire);
      if (tmp1 < 3) {
        ok = 1;
        // break;	
      }	
    // }      // i loop: end
    if (ok==0) return NULL;

    flag1.store(3, memory_order_release);

    __fence_var.fetch_add(0, memory_order_acq_rel);

    int tmp2 = flag2.load(memory_order_acquire);
    if (tmp2 == 1) {
      flag1.store(2, memory_order_release);

      __fence_var.fetch_add(0, memory_order_acq_rel);

      ok =0;
      // for (int i=0; i<LOOP; i++) {      // second i loop: begin
        int tmp3 = flag2.load(memory_order_acquire);
        if (tmp3 == 4) {
          ok = 1;
          // break;		
        }
      // }                                 // second i loop: end
      if (ok==0) return NULL;
    }
    
    flag1.store(4, memory_order_release);

    ok = 0;
    // for (int i=0; i<LOOP; i++) {        // third i loop: begin
      int tmp4 = flag2.load(memory_order_acquire);
      if (tmp4 < 2) {
        ok = 1;
        // break;	
      }
    // }                                   // third i loop: end
    if (ok==0) return NULL;

    // critical section
    var.store(1, memory_order_release);
    int tmp5 = var.load(memory_order_acquire);
    assert(tmp5==1);

    ok = 0;
    // for (int i=0; i<LOOP; i++) {        // fourth i loop: begin
      int tmp6 = flag2.load(memory_order_acquire);
      int tmp7 = flag2.load(memory_order_acquire);
      if (2 > tmp6 || tmp7 > 3) {
        ok = 1;
        // break;	
      }
    // }                                   // fourth i loop: end 
    if (ok==0) return NULL;

    flag1.store(0, memory_order_release);
  // }     // j loop: end
  return NULL;
}

void *p2(void *arg)
{
  int ok;
  // for (int j=0; j<OUTER_LOOP; j++) {      // outer j loop: begin
    flag2.store(1, memory_order_release);

    ok = 0;
    // for (int i=0; i<LOOP; i++) {          // first i loop: begin
    int tmp1 = flag1.load(memory_order_acquire);
      if (tmp1 < 3) {
        ok = 1;
        // break;	
      }	
    // }                                     // first i loop: end
    if (ok==0) return NULL;

    flag2.store(3, memory_order_release);

    __fence_var.fetch_add(0, memory_order_acq_rel);

    int tmp2 = flag1.load(memory_order_acquire);
    if (tmp2 == 1) {
      flag2.store(2, memory_order_release);

      __fence_var.fetch_add(0, memory_order_acq_rel);

      ok =0;
      // for (int i=0; i<LOOP; i++) {          // second i loop: begin
        int tmp3 = flag1.load(memory_order_acquire);
        if (tmp3 == 4) {
          ok = 1;
          // break;		
        }
      // }                                     // second i loop: begin
      if (ok==0) return NULL;

    }

    flag2.store(4, memory_order_release);

    ok = 0;
    // for (int i=0; i<LOOP; i++) {            // third i loop: begin
      int tmp4 = flag1.load(memory_order_acquire);
      if (tmp4 < 2) {
        ok = 1;
        // break;	
      }
    // }                                       // third i loop: end
    if (ok==0) return NULL;

    // critical section
    var.store(2, memory_order_release);
    int tmp5 = var.load(memory_order_acquire);
    assert(tmp5==2);

    ok = 0;
    // for (int i=0; i<LOOP; i++) {            // fourth i loop: begin
      int tmp6 = flag1.load(memory_order_acquire);
      int tmp7 = flag1.load(memory_order_acquire);
      if (2 > tmp6 || tmp7 > 3) {
        ok = 1;
        // break;	
      }
    // }                                       // fourth i loop: end
    if (ok==0) return NULL;

    flag2.store(0, memory_order_release);
  // }                                         // outer j loop: end
  return NULL;
}

int main()
{
	pthread_t a, b;

  pthread_create(&a, NULL, p1, NULL);
  pthread_create(&b, NULL, p2, NULL);

  pthread_join(a, NULL);
  pthread_join(b, NULL);

 return 0;
}

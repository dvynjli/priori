/* Copyright (C) 2018 Tuan Phong Ngo 
 * This benchmark is part of TRACER
 */

/*
 * Dekker's critical section algorithm, implemented with fences.
 *
 * URL:
 *   http://www.justsoftwaresolutions.co.uk/threading/
 */

#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

#define LOOP 5
#define OUTER_LOOP 2
#define LB LOOP * OUTER_LOOP

// shared variables
atomic_int flag0;
atomic_int flag1;
atomic_int turn;

atomic_int var;

atomic_int __fence_var;

void* p0(void *arg)
{
  int ok1, ok2;
//   for (int jj=0; jj<OUTER_LOOP; jj++) {			// outer jj loop: begin
    flag0.store(1, memory_order_release);
    
    __fence_var.fetch_add(0, memory_order_acq_rel);
    
    ok1 = 0;
    // for (int i = 0; i<LOOP; i++) {				// inner i loop: begin
	  int tmp1 = flag1.load(memory_order_acquire);
      if (tmp1) {
		int tmp2 = turn.load(memory_order_acquire);
        if (tmp2 != 0)
        {
          flag0.store(0, memory_order_release);
          ok2 = 0;
        //   for (int j=0; j<LOOP; j++) {			// inner j loop: begin
		  	int tmp3 = turn.load(memory_order_acquire);
            if (tmp3 == 0) {
              ok2 = 1;
            //   break;
            }
        //   }										// outer jj loop: end
          if (ok2 == 0) return NULL;
          flag0.store(1, memory_order_release);
          __fence_var.fetch_add(0, memory_order_acq_rel);
        }
        
      } else {
        ok1 = 1;
        // break;
      }
    // }											// outer jj loop: end
    if (ok1 == 0) return NULL;
    
    // critical section
    var.store(1, memory_order_release);
	int tmp4 = var.load(memory_order_acquire);
    assert(tmp4==1);
    
    turn.store(1, memory_order_release);
    flag0.store(0, memory_order_release);
//   	}											// outer jj loop: end
  return NULL;
}

void* p1(void *arg)
{
  int ok1, ok2;
  
//   for (int jj=0; jj<OUTER_LOOP; jj++) {			// outer jj loop: begin
    flag1.store(1, memory_order_release);
    __fence_var.fetch_add(0, memory_order_acq_rel);
    
    ok1 = 0;
    // for (int i=0; i<LOOP; i++) {				// inner i loop: begin
	  int tmp = flag0.load(memory_order_acquire);
      if (tmp) {
		tmp = turn.load(memory_order_acquire);
        if (tmp != 1)
        {
          flag1.store(0, memory_order_release);
          ok2 = 0;
        //   for (int j=0; j<LOOP; j++) {			// inner j loop: begin
			tmp = turn.load(memory_order_acquire);
            if (tmp == 1) {
              ok2 = 1;
            //   break;
            }
        //   }										// inner j loop: end
          if (ok2==0) return NULL;
          flag1.store(1, memory_order_release);
          __fence_var.fetch_add(0, memory_order_acq_rel);
        }
      } else {
        ok1 = 1;
        // break;
      }
    // }											// inner i loop: end
    if (ok1==0) return NULL;
    
    // critical section
    var.store(2, memory_order_release);
	tmp = var.load(memory_order_acquire);
    assert(tmp==2);
    
    turn.store(0, memory_order_release);
    flag1.store(0, memory_order_release);
//   }												// outer jj loop: end
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
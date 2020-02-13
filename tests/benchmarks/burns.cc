/* Copyright (C) 2018 Tuan Phong Ngo 
 * This benchmark is part of TRACER
 */

/*
 * Burn's critical section algorithm, implemented with fences.
 *
 * URL:
 *   http://www.cs.yale.edu/homes/aspnes/pinewiki/attachments/MutualExclusion/burns-lynch-1993.pdf
 */

#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

#define LOOP 10
#define N 2

// shared variables
atomic_int flag0, flag1;
atomic_int var;

atomic_int __fence_var;

void* t0(void *arg)
{
  	int tid = 0;
  	int ok = 0;
  	int restart = 0;
  
  	// for (int j=0; j<LOOP; j++) {					// outer j loop: begin
    	// down
    	flag0.store(0, memory_order_release);
	    // for (int i=0; i<tid; i++) {				// first i loop: begin
	    //   ok = 0;
	    //   for (int jj=0; jj<LOOP; jj++) {		// inner jj loop: begin
		  	// int tmp1 = flag0.load(memory_order_acquire);
	        // if (tmp1==0) {
	        //   ok = 1;
	        //   break;
	        // }
	    //   }										// inner jj loop: end
	    //   if (ok==0) return;
	    // }										// first i loop: end
	    // up
		flag0.store(1, memory_order_release);
	    // atomic_store_explicit(&flags[tid], 1, memory_order_release);
	  
	  	__fence_var.fetch_add(0, memory_order_acq_rel);

	  	// for (int i=0; i<tid; i++) {				// second i loop: begin
	    //   ok = 0;
	    //   for (int jj=0; jj<LOOP; jj++) {		// second jj loop: begin
		  	// int tmp2 = flag0.load(memory_order_acquire);
	        // if (tmp2==0) {
	        //   ok = 1;
	        //   break;
	        // }
	    //   }										// second jj loop: end
	    //   if (ok==0) {
	      	// restart = 1;
	      	// break;
	    //   };
	    // }										// second i loop: end
	    if (restart==0) {
	    	ok = 1;
	    	// break;
	    } else ok = 0;
  	// }											// outer j loop: end

  	if (ok==0) return NULL;


  	// for (int i=tid+1; i<N; i++) {				// third i loop: begin (for threads above the current one)
      	ok = 0;
      	// for (int jj=0; jj<LOOP; jj++) {			// third j loop: begin
		  	int tmp3 = flag1.load(memory_order_acquire);
        	if (tmp3==0) {
          		ok = 1;
          		// break;
        	}
      	// }										// third j loop: end
      	if (ok==0) return NULL;
	// }											// third i loop: end
  
  	// critical section
	var.store(tid, memory_order_release);
	int tmp4 = var.load(memory_order_acquire);
    assert(tmp4==tid);
  
  	flag0.store(0, memory_order_release);
	return NULL;
}



void* t1(void *arg)
{
  	int tid = 1;
  	int ok = 0;
  	int restart = 0;
  
  	// for (int j=0; j<LOOP; j++) {				// outer j loop: begin
    	// down
    	flag0.store(0, memory_order_release);
	    // for (int i=0; i<tid; i++) {				// first i loop: begin
	      ok = 0;
	    //   for (int jj=0; jj<LOOP; jj++) {		// first jj loop: begin
		  	int tmp1 = flag0.load(memory_order_acquire);
	        if (tmp1==0) {
	          ok = 1;
	        //   break;
	        }
	    //   }										// first jj loop: end
	      if (ok==0) return NULL;
	    // }										// first i loop: end
	    // up
	    flag1.store(1, memory_order_release);
	  
	  	__fence_var.fetch_add(0, memory_order_acq_rel);

	  	// for (int i=0; i<tid; i++) {				// second i loop: begin
	      ok = 0;
	    //   for (int jj=0; jj<LOOP; jj++) {		// second jj loop: begin
		  	int tmp2 = flag0.load(memory_order_acquire);
	        if (tmp2==0) {
	          ok = 1;
	        //   break;
	        }
	    //   }										// second jj loop: end
	      if (ok==0) {
	      	restart = 1;
	      	// break;
	      };
	    // }										// second i loop: end
	    if (restart==0) {
	    	ok = 1;
	    	// break;
	    } else ok = 0;
  	// }											// outer j loop: end

  	if (ok==0) return NULL;


  	// for (int i=tid+1; i<N; i++) {				// loop will not run
      	// ok = 0;
      	// for (int jj=0; jj<LOOP; jj++) {
        	// if (atomic_load_explicit(&flags[i], memory_order_acquire)==0) {
          		// ok = 1;
          		// break;
        	// }
      	// }
      	// if (ok==0) return;
	// }
  
  	// critical section
	var.store(tid, memory_order_release);
	int tmp4=var.load(memory_order_acquire);
    assert(tmp4==tid);
  
  	flag1.store(0, memory_order_release);
	return NULL;
}


int main()
{
  	pthread_t ts0, ts1;
	  
	pthread_create(&ts0, NULL, t0, NULL);
	pthread_create(&ts1, NULL, t1, NULL);
	
  	pthread_join(ts0, NULL);
  	pthread_join(ts1, NULL);
  
  	return 0;
}
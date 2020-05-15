/* Copyright (C) 2018 Tuan Phong Ngo 
 * This benchmark is part of TRACER
 */

/*
 * PostgreSQL bug under Power.
 *
 * URL: Listing 1.1 in
 *   https://arxiv.org/pdf/1207.7264.pdf
 */

#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

#define LOOP 10
#define N 2

// shared variables
atomic_int latch0, latch1;
atomic_int flag0, flag1;
atomic_int var;

atomic_int __fence_var;

void *t0(void *arg)
{
  	int tid = 0;
  	int ok = 0;
  	
    // for (int i=0; i<LOOP; i++) {					// loop i: begin
		int tmp1 = latch0.load(memory_order_acquire);
    	if (tmp1==1) {
    		ok = 1;
    		// break;
    	}    
    // }											// loop i: end
    
    if (ok==0) return NULL;

	int tmp2 = latch0.load(memory_order_acquire);
	int tmp3 = flag0.load(memory_order_acquire);
	assert(tmp2==0 || tmp3==1);

	latch0.store(0, memory_order_release);

	int tmp4 = flag0.load(memory_order_acquire);
	if (tmp4==1) {
		flag0.store(0, memory_order_release);
		flag1.store(1, memory_order_release);
		latch1.store(1, memory_order_release);
	} 
	return NULL;
}

void *t1(void *arg)
{
  	int tid = 1;
  	int ok = 0;
  	
    // for (int i=0; i<LOOP; i++) {				// loop i: begin
		int tmp1 = latch1.load(memory_order_acquire);
    	if (tmp1==1) {
    		ok = 1;
    		// break;
    	}    
    // }											// loop i: end
    
    if (ok==0) return NULL;

	int tmp2 = latch1.load(memory_order_acquire);
	int tmp3 = flag1.load(memory_order_acquire);
	assert(tmp2==0 || tmp3==1);

	latch1.store(0, memory_order_release);

	int tmp4 = flag1.load(memory_order_acquire);
	if (tmp4==1) {
		flag1.store(0, memory_order_release);
		flag0.store(1, memory_order_release);
		latch0.store(1, memory_order_release);	
	}
	return NULL;
}

/* 
void t(void *arg)
{
  	int tid = *((int *)arg);
  	int ok = 0;
  	
    for (int i=0; i<LOOP; i++) {
    	if (atomic_load_explicit(&latch[tid], memory_order_acquire)==1) {
    		ok = 1;
    		break;
    	}    
    }
    
    if (ok==0) return;

	MODEL_ASSERT(atomic_load_explicit(&latch[tid], memory_order_acquire)==0 ||
				 atomic_load_explicit(&flag[tid], memory_order_acquire)==1);

	atomic_store_explicit(&latch[tid], 0, memory_order_release);

	if (atomic_load_explicit(&flag[tid], memory_order_acquire)==1) {
		atomic_store_explicit(&flag[tid], 0, memory_order_release);
		atomic_store_explicit(&flag[(tid+1)%N], 1, memory_order_release);
		atomic_store_explicit(&latch[(tid+1)%N], 1, memory_order_release);	
	} 
  
} 
*/


int main(int argc, char **argv)
{
  	pthread_t ts0,ts1;

	latch0.store(1, memory_order_release);
	flag0.store(1, memory_order_release);
	  
	pthread_create(&ts0, NULL, t0, NULL);
	pthread_create(&ts1, NULL, t1, NULL);
	
  
  	pthread_join(ts0, NULL);
  	pthread_join(ts1, NULL);
  
  	return 0;
}
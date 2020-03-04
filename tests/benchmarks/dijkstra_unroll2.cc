/* Copyright (C) 2018 Tuan Phong Ngo 
 * This benchmark is part of TRACER
 */

/*
 * Dijkstra's critical section algorithm, implemented with fences.
 *
 * URL:
 *   https://www.eecs.yorku.ca/course_archive/2007-08/W/6117/DijMutexNotes.pdf
 */


#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>


#define LOOP 2 		// TODO: the loop has to be unrolled 10 times
#define N 2



using namespace std;

// shared variables
atomic_int interested0, interested1;;
atomic_int passed0, passed1;
atomic_int k;

atomic_int var;

atomic_int __fence_var;

void* t0(void *arg)
{
	int ok = 0;
	int done, _k;
 	int tid = 0;
  	
  	interested0.store(1, memory_order_release);

  	done = 0;

  	// for (int jj=0; jj<LOOP; jj++) {					// jj loop: begin, itr0
  		// for (int j=0; j<LOOP; j++) {					// j loop: begin, itr0
  			_k = k.load(memory_order_acquire); 
	    	if (_k==tid) {
	    		ok = 1;
	    		// break;
	    	}
			else {
				int tmp1;
				if (_k==0) tmp1 = interested0.load(memory_order_acquire);
				else tmp1 = interested1.load(memory_order_acquire);
				if (tmp1==0)
					k.store(tid, memory_order_release);
														// j loop: itr1
				_k = k.load(memory_order_acquire); 
				if (_k==tid) {
					ok = 1;
					// break;
				}
				else {
					int tmp1;
					if (_k==0) tmp1 = interested0.load(memory_order_acquire);
					else tmp1 = interested1.load(memory_order_acquire);
					if (tmp1==0)
						k.store(tid, memory_order_release);
					
				}
			}
	    // }											// j loop: end
	  
	  	if (ok==0) return NULL;

	  	passed0.store(1, memory_order_release);
		__fence_var.fetch_add(0, memory_order_acq_rel);

	  	done = 1;

	  	int tmp2 = passed1.load(memory_order_acquire);
		if (tmp2 == 1) {
			passed0.store(0, memory_order_release);
			done = 0;
		}

	  	if (done==1) {
	  		ok = 1; 
	  		// break;
	  	} else {
	  		ok = 0;
														// j loop: itr1
			// for (int j=0; j<LOOP; j++) {				// j loop: begin, itr0
  			_k = k.load(memory_order_acquire); 
	    	if (_k==tid) {
	    		ok = 1;
	    		// break;
	    	}
			else {
				int tmp1;
				if (_k==0) tmp1 = interested0.load(memory_order_acquire);
				else tmp1 = interested1.load(memory_order_acquire);
				if (tmp1==0)
					k.store(tid, memory_order_release);
														// j loop: itr1
				_k = k.load(memory_order_acquire); 
				if (_k==tid) {
					ok = 1;
					// break;
				}
				else {
					int tmp1;
					if (_k==0) tmp1 = interested0.load(memory_order_acquire);
					else tmp1 = interested1.load(memory_order_acquire);
					if (tmp1==0)
						k.store(tid, memory_order_release);
					
				}
			}
	    // }											// j loop: end
	  
			if (ok==0) return NULL;

			passed0.store(1, memory_order_release);
			__fence_var.fetch_add(0, memory_order_acq_rel);

			done = 1;

			int tmp2 = passed1.load(memory_order_acquire);
			if (tmp2 == 1) {
				passed0.store(0, memory_order_release);
				done = 0;
			}

			if (done==1) {
				ok = 1; 
				// break;
			} else 
				ok = 0;
		}
  	// }												// jj loop: end
    
  	if (ok==0) return NULL;

  	// critical section
	var.store(tid,memory_order_release);
	int tmp3 = var.load(memory_order_acquire);
	assert(tmp3 == tid);
    
	passed0.store(0, memory_order_release);
	interested0.store(0, memory_order_release);

	return NULL;
}


void* t1(void *arg)
{
	int ok = 0;
	int done, _k;
 	int tid = 1;
  	
  	interested1.store(1, memory_order_release);

  	done = 0;

  	// for (int jj=0; jj<LOOP; jj++) {					// jj loop: begin, itr0
  		// for (int j=0; j<LOOP; j++) {					// j loop: begin, itr0
  			_k = k.load(memory_order_acquire); 
	    	if (_k==tid) {
	    		ok = 1;
	    		// break;
	    	}
			else {
				int tmp1;
				if (_k==0) tmp1 = interested0.load(memory_order_acquire);
				else tmp1 = interested1.load(memory_order_acquire);
				if (tmp1==0)
					k.store(tid, memory_order_release);
														// j loop: itr1
				_k = k.load(memory_order_acquire); 
				if (_k==tid) {
					ok = 1;
					// break;
				}
				else {
					int tmp1;
					if (_k==0) tmp1 = interested0.load(memory_order_acquire);
					else tmp1 = interested1.load(memory_order_acquire);
					if (tmp1==0)
						k.store(tid, memory_order_release);
				}
			}
	    // }											// j loop: end
	  
	  	if (ok==0) return NULL;

	  	passed1.store(1, memory_order_release);
		__fence_var.fetch_add(0, memory_order_acq_rel);

	  	done = 1;

	  	int tmp2 = passed0.load(memory_order_acquire);
		if (tmp2 == 1) {
			passed1.store(0, memory_order_release);
			done = 0;
		}

	  	if (done==1) {
	  		ok = 1; 
	  		// break;
	  	} else {
	  		ok = 0;
			  											// jj loop: itr1
			_k = k.load(memory_order_acquire); 
	    	if (_k==tid) {
	    		ok = 1;
	    		// break;
	    	}
			else {
				int tmp1;
				if (_k==0) tmp1 = interested0.load(memory_order_acquire);
				else tmp1 = interested1.load(memory_order_acquire);
				if (tmp1==0)
					k.store(tid, memory_order_release);
														// j loop: itr1
				_k = k.load(memory_order_acquire); 
				if (_k==tid) {
					ok = 1;
					// break;
				}
				else {
					int tmp1;
					if (_k==0) tmp1 = interested0.load(memory_order_acquire);
					else tmp1 = interested1.load(memory_order_acquire);
					if (tmp1==0)
						k.store(tid, memory_order_release);
				}
			}
	    // }											// j loop: end
	  
			if (ok==0) return NULL;

			passed1.store(1, memory_order_release);
			__fence_var.fetch_add(0, memory_order_acq_rel);

			done = 1;

			int tmp2 = passed0.load(memory_order_acquire);
			if (tmp2 == 1) {
				passed1.store(0, memory_order_release);
				done = 0;
			}

			if (done==1) {
				ok = 1; 
				// break;
			} else 
				ok = 0;
		}
  	// }
    
  	if (ok==0) return NULL;

  	// critical section
	var.store(tid,memory_order_release);
	int tmp3 = var.load(memory_order_acquire);
	assert(tmp3 == tid);
    
	passed1.store(0, memory_order_release);
	interested1.store(0, memory_order_release);

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
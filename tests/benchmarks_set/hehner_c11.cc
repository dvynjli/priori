/* Implemented under the RA semantics
 * This benchmark is part of TRACER
 * Need to use fences under RA to avoid 
 * a violation of mutual execlusion property
 * Get orgiral source code from: 
 * https://github.com/pramalhe/ConcurrencyFreaks/blob/master/C11/papers/cralgorithm/HehnerC11.c
 */

// Eric C. R. Hehner and R. K. Shyamasundar, An Implementation of P and V, Information Processing Letters, 1981, 12(4),
// pp. 196-197

#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

#define LOOP 5
#define N 2

// enum { MAX_TICKET = INTPTR_MAX };

// shared variables
atomic_int ticket0, ticket1;
atomic_int var;

atomic_int __fence_var;

void* Worker0(void *arg)
{
  	int id = 0;
    int ok;
  
    ticket0.store(0, memory_order_release);
  
    __fence_var.fetch_add(0, memory_order_acq_rel);

    int max = 0;
    // for (int j=0; j<N; j+=1) {				// first j loop: begin (unrolled)
	  int v = ticket0.load(memory_order_acquire);
	  if (max < v && v != INT32_MAX ) max = v;
	  v = ticket1.load(memory_order_acquire);
      if (max < v && v != INT32_MAX ) max = v;
    // } 										// first j loop: end
    max += 1;
  
    ticket0.store(max, memory_order_release);

    __fence_var.fetch_add(0, memory_order_acq_rel);
  
    // for (int j = 0; j<N; j+=1 ) {			// second j loop: begin (unrolled)
	  int j = 0;
      ok = 0;
    //   for (int jj=0; jj<LOOP; jj++) {		// first jj loop: begin (for j=0)
	  	int tmp1 = ticket0.load(memory_order_acquire);
	  	if (!(tmp1 < max)) {
		  // !(j < id) is true for j = 0. can skip the check
		  if (!(j < id)) {
			ok = 1;
			// break;
		  }
		  // else is never reachable. can skip
		  else {
		    int tmp2 = ticket0.load(memory_order_acquire);
			if (!(tmp2 == max)) {
				ok = 1;
				// break;
			}
		  }
		}
	//   }										// first jj loop: end (for j=0)
      if (ok==0) return NULL;
	  j = 1;									// second j loop: begin (for j=1)
	  ok = 0;
    //   for (int jj=0; jj<LOOP; jj++) {		// first jj loop: begin (for j=1)
	  	tmp1 = ticket1.load(memory_order_acquire);
	  	if (!(tmp1 < max)) {
		  // !(j < id) is true for j = 0. can skip the check
		  if (!(j < id)) {
			ok = 1;
			// break;
		  }
		  // else is never reachable. can skip
		  else {
		    int tmp2 = ticket1.load(memory_order_acquire);
			if (!(tmp2 == max)) {
				ok = 1;
				// break;
			}
		  }
		}
	//   }										// first jj loop: end (for j=1)
      if (ok==0) return NULL;
    // }										// second j loop: end
  
    // critical section
    var.store(id, memory_order_release);
	int tmp3 = var.load(memory_order_acquire);
    assert(tmp3==id);

    ticket0.store(INT32_MAX, memory_order_release);
	
    return NULL;

} // Worker


void* Worker1(void *arg)
{
  	int id = 1;
    int ok;
  
    ticket1.store(0, memory_order_release);
  
    __fence_var.fetch_add(0, memory_order_acq_rel);

    int max = 0;
    // for (int j=0; j<N; j+=1) {				// first j loop: begin (unrolled)
      int v = ticket0.load(memory_order_acquire);
      if (max < v && v != INT32_MAX ) max = v;
	  v = ticket1.load(memory_order_acquire);
      if (max < v && v != INT32_MAX ) max = v;
    // } 										// first j loop: end 
    max += 1;
  
    ticket1.store(max, memory_order_release);

    __fence_var.fetch_add(0, memory_order_acq_rel);
  
    // for (int j = 0; j<N; j+=1 ) {			// second j loop: begin (unrolled)
	  int j = 0;
      ok = 0;
    //   for (int jj=0; jj<LOOP; jj++) {		// first jj loop: begin (for j=0)
        int tmp1 = ticket0.load(memory_order_acquire);
	  	if (!(tmp1 < max)) {
		  // !(j < id) is false for j = 0. This branch is never reachable
		  if (!(j < id)) {
			ok = 1;
			// break;
		  }
		  // else brach will always execute.
		  else {
		    int tmp2 = ticket0.load(memory_order_acquire);
			if (!(tmp2 == max)) {
				ok = 1;
				// break;
			}
		  }
		}
    //   }										// first jj loop: end (for j=0)
      if (ok==0) return NULL;
	  j = 1;
      ok = 0;
    //   for (int jj=0; jj<LOOP; jj++) {		// first jj loop: begin (for j=1)
	    tmp1 = ticket1.load(memory_order_acquire);
	  	if (!(tmp1 < max)) {
		  // !(j < id) is true for j = 0. can skip the check
		  if (!(j < id)) {
			ok = 1;
			// break;
		  }
		  // else is never reachable. can skip
		  else {
		    int tmp2 = ticket1.load(memory_order_acquire);
			if (!(tmp2 == max)) {
				ok = 1;
				// break;
			}
		  }
		}										// first jj loop: end (for j=1)
      if (ok==0) return NULL;
    // }										// second j loop: end 
  
    // critical section
	var.store(id, memory_order_release);
	int tmp3 = var.load(memory_order_acquire);
    assert(tmp3==id);

    ticket1.store(INT32_MAX, memory_order_release);
	
    return NULL;

} // Worker


int main()
{
  	pthread_t ts0, ts1;

  	ticket0.store(INT32_MAX, memory_order_release);
  	ticket1.store(INT32_MAX, memory_order_release);
  	  
  	pthread_create(&ts0, NULL, Worker0, NULL);
  	pthread_create(&ts1, NULL, Worker1, NULL);
  	
  	pthread_join(ts0, NULL);
  	pthread_join(ts1, NULL);
  	
  	return 0;
}

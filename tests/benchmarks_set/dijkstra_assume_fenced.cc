#include "../verify.h"


#define LOOP 10 		// TODO: the loop has to be unrolled 10 times
#define N 2

// shared variables
atomic<int> interested0, interested1;;
atomic<int> passed0, passed1;
atomic<int> k;

atomic<int> var;

atomic<int> __fence_var;

void* t0(void *arg)
{
	int done, _k;
 	int tid = 0;
  	
  	interested0.store(1, memory_order_release);
  	done = 0;

  	// for (int jj=0; jj<LOOP; jj++) {
  		// for (int j=0; j<LOOP; j++) {
			_k = k.load(memory_order_acquire);	  
			if (_k!=tid) {
				int tmp1;
				tmp1 = interested1.load(memory_order_acquire);
				if (tmp1==0)
					k.store(tid, memory_order_release);
				
				// second iteration of j loop
				_k = k.load(memory_order_acquire);	  
				if (_k!=tid) {
					int tmp1;
					tmp1 = interested1.load(memory_order_acquire);
					if (tmp1==0)
						k.store(tid, memory_order_release);
				}
			}
			assume(_k == tid);
	    // }
	  
	  	passed0.store(1, memory_order_release);
		__fence_var.fetch_add(0, memory_order_acq_rel);

	  	done = 1;

	  	int tmp2 = passed1.load(memory_order_acquire);
		if (tmp2 == 1) {
			passed0.store(0, memory_order_release);
			done = 0;
		}

	assume(done == 1);
  	// }
    

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
	int done, _k;
 	int tid = 1;
  	
  	interested1.store(1, memory_order_release);

  	done = 0;

  	// for (int jj=0; jj<LOOP; jj++) {
  		// for (int j=0; j<LOOP; j++) {
  			_k = k.load(memory_order_acquire);
			if (_k != tid) {
				int tmp1 = interested0.load(memory_order_acquire);
				if (tmp1==0)
					k.store(tid, memory_order_release);
				// second iteration of j loop
				_k = k.load(memory_order_acquire);
				if (_k != tid) {
					int tmp1 = interested0.load(memory_order_acquire);
					if (tmp1==0)
						k.store(tid, memory_order_release);
				}
			}
			assume(_k == tid);
	    // }

	  	passed1.store(1, memory_order_release);
		__fence_var.fetch_add(0, memory_order_acq_rel);

	  	done = 1;

	  	int tmp2 = passed0.load(memory_order_acquire);
		if (tmp2 == 1) {
			passed1.store(0, memory_order_release);
			done = 0;
		}

	assume (done == 1);
  	// }

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
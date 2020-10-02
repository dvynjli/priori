#include "../verify.h"

// shared variables
atomic<int> x, y; 
atomic<int> b1, b2;

atomic<int> _cc_X;

atomic_int __fence_var;


void* t0(void *arg)
{
	int ry1 = -1, ry2 = -1, ry3 = -1, ry4 = -1, rx = -1, rb2 = -1;
	// first while(1) iter 1
	// second while(1) iter 1
	b1.store(1, memory_order_release);
	x.store(1, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	ry1 = y.load(memory_order_acquire);
	if (ry1 != 0) {
		b1.store(0, memory_order_release);
		ry2 = y.load(memory_order_acquire);
		// while (ry2 != 0)
		if (ry2 != 0) 
			ry2 = y.load(memory_order_release);
		assume(ry2 == 0);
		// end while line 23
		// continue; 	// since we have only one iterartion here, exit the program
		return NULL;
	}
	y.store(1, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	rx = x.load(memory_order_acquire);
	if (rx != 1) {
		b1.store(0, memory_order_release);
		rb2 = b2.load(memory_order_acquire);
		// while (rb2 >= 1)
		if (rb2 >= 1) {
			rb2 = b2.load(memory_order_acquire);
		}
		assume(rb2 < 1);
		// end while line 36
		__fence_var.fetch_add(0, memory_order_acq_rel);
		ry3 = y.load(memory_order_acquire);
		if (ry3 != 1){
			ry4 = y.load(memory_order_acquire);
            // while (ry4 != 0)
            if (ry4 != 0) {
				ry4 = y.load(memory_order_acquire);
			}
			assume(ry4 == 0);
			// end while line 45
			// continue; 	// since we have only one iterartion here, exit the program
			return NULL;
		}
	}
	// break;		// from second while(1) loop
	// end second while(1) loop
	_cc_X.store(0, memory_order_release);
	rx = _cc_X.load(memory_order_acquire);
	assert(rx <= 0);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	y.store(0, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	b1.store(0, memory_order_release);

	// end first while(1) loop
	return NULL;
}

void *t1(void *arg)
{
	int ry1 = -1, ry2 = -1, ry3 = -1, ry4 = -1, rx = -1, rb1 = -1;
	// first while(1) loop
	// second while(1) loop
	b2.store(1,memory_order_release);
	x.store(2, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	ry1 = y.load(memory_order_acquire);
	if (ry1 != 0) {
		b2.store(0, memory_order_release);
		ry2 = y.load(memory_order_acquire);
        // while (ry2 != 0)
        if (ry2 != 0) {
			ry2 = y.load(memory_order_acquire);
		}
		assume(ry2 == 0);
		// end while line 78
		// continue; 	// since we have only one iterartion here, exit the program
		return NULL;
	}
	y.store(2, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	rx = x.load(memory_order_acquire);
	if (rx != 2) {
		b2.store(0, memory_order_release);
		rb1 = b1.load(memory_order_acquire);
		// while (rb1 >= 1)
		if (rb1 >= 1) {
			rb1 = b1.load(memory_order_acquire);
		}
		assume(rb1 < 1);
		// end while line 92
		// __fence_var.fetch_add(0, memory_order_acq_rel);
		ry3 = y.load(memory_order_acquire);
		if (ry3 != 2) {
			ry4 = y.load(memory_order_acquire);
			// while (ry4 != 0)
			if (ry4 != 0) {
				ry4 = y.load(memory_order_acquire);
			}
			assume(ry4 == 0);
			// continue; 	// since we have only one iterartion here, exit the program
			return NULL;
		}
	}
	// break;		// from second while(1) loop
	// end second while(1) loop
	_cc_X.store(1, memory_order_release);
	rx = _cc_X.load(memory_order_acquire);
	assert(rx >= 1);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	y.store(0, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	b1.store(0, memory_order_release);

	// end first while(1) loop
	return NULL;
}

int main(int argc, char **argv)
{
	__fence_var.store(0, memory_order_release);
  	pthread_t ts0, ts1;
	  
	pthread_create(&ts0, NULL, t0, NULL);
	pthread_create(&ts1, NULL, t1, NULL);
	  
  	pthread_join(ts0, NULL);
  	pthread_join(ts1, NULL);
  	  
  	return 0;
}

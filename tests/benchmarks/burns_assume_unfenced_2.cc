#include "../verify.h"

// shared variables
atomic<int> flag1, flag2;
atomic<int> _cc_x;

// atomic<int> __fence_var;

void* t0(void *arg)
{
	flag1.store(0, memory_order_release);
	flag1.store(1, memory_order_release);

	int rflag2 = flag2.load(memory_order_acquire);
	assume(rflag2!=1);

	// critical section
	_cc_x.store(0, memory_order_release);
	int rx = _cc_x.load(memory_order_acquire);
	assert(rx<=0);

	flag1.store(0, memory_order_release);


	// Unrolled #iter=2
	flag1.store(0, memory_order_release);
	flag1.store(1, memory_order_release);

	rflag2 = flag2.load(memory_order_acquire);
	assume(rflag2!=1);

	// critical section
	_cc_x.store(0, memory_order_release);
	rx = _cc_x.load(memory_order_acquire);
	assert(rx<=0);

	flag1.store(0, memory_order_release);
	return NULL;
}



void* t1(void *arg)
{
	flag2.store(0, memory_order_release);
	int rflag1 = flag1.load(memory_order_acquire);
	assume(rflag1!=1);
	flag2.store(1, memory_order_release);
	rflag1 = flag1.load(memory_order_acquire);
	assume(rflag1!=1);

	// critical section
	_cc_x.store(1, memory_order_release);
	int rx = _cc_x.load(memory_order_acquire);
	assert(rx>=1);

	flag1.store(0, memory_order_release);


	// Unrolled #iter=2
	flag2.store(0, memory_order_release);
	rflag1 = flag1.load(memory_order_acquire);
	assume(rflag1!=1);
	flag2.store(1, memory_order_release);
	rflag1 = flag1.load(memory_order_acquire);
	assume(rflag1!=1);

	// critical section
	_cc_x.store(1, memory_order_release);
	rx = _cc_x.load(memory_order_acquire);
	assert(rx>=1);

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
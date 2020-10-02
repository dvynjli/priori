#include "../verify.h"

// shared variables
atomic<int> x, y;
atomic<int> _cc_X;

// atomic<int> __fence_var;

void* t0(void *arg)
{
	x.store(1, memory_order_release);
	int ry = y.load(memory_order_acquire);
	assume (ry == 0);
	
	_cc_X.store(0, memory_order_release);
	int rX = _cc_X.load(memory_order_acquire);
	assert(rX<=0);
	
	x.store(0, memory_order_release);
	return NULL;
}



void* t1(void *arg)
{
	y.store(1, memory_order_release);
	int rx = x.load(memory_order_acquire);
	assume(rx==0);

	_cc_X.store(1, memory_order_release);
	int rX = _cc_X.load(memory_order_acquire);
	assert(rX>=1);

	y.store(0, memory_order_release);
	return NULL;
}


int main()
{
	x.store(0, memory_order_release);
	y.store(0, memory_order_release);
  	pthread_t ts0, ts1;
	  
	pthread_create(&ts0, NULL, t0, NULL);
	pthread_create(&ts1, NULL, t1, NULL);
	
  	pthread_join(ts0, NULL);
  	pthread_join(ts1, NULL);
  
  	return 0;
}

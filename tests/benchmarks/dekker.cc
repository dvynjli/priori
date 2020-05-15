#include "../verify.h"

// shared variables
atomic<int> flag1;
atomic<int> flag2;
atomic<int> turn;
atomic<int> _cc_x;

// atomic_int __fence_var;

void* p0(void *arg)
{
	int rflag2 = -1, rturn_1 = -1, rturn_2 = -1, rx = -1;

	flag1.store(1, memory_order_release);
	rflag2 = flag2.load(memory_order_acquire);

	// while (rflag2 >=1) {
	if (rflag2 >=1) {
		rturn_1 = turn.load(memory_order_acquire);
		if (rturn_1 != 0) {
			flag1.store(0, memory_order_release);
			rturn_2 = turn.load(memory_order_acquire);
			assume (rturn_2 == 0);
			flag1.store(1, memory_order_release);
		}
		rflag2 = flag2.load(memory_order_acquire);
	}
	// } // end while 
	assume (rflag2 < 1);

	// critical section
	_cc_x.store(0, memory_order_release);
	rx = _cc_x.load(memory_order_acquire);
	assert(rx <= 0);

	turn.store(1, memory_order_release);
	flag1.store(0, memory_order_release);
	return NULL;
}

void* p1(void *arg)
{
	int rflag1 = -1, rturn_1 = -1, rturn_2 = -1, rx = -1;
	flag2.store(1, memory_order_release);
	rflag1 = flag1.load(memory_order_acquire);

	// while (rflag1 >= 1) {
	if (rflag1 >= 1) {
		rturn_1 = turn.load(memory_order_acquire);
		if (rturn_1 != 1) {
			flag2.store(0, memory_order_release);
			rturn_2 = turn.load(memory_order_acquire);
			assume(rturn_2 == 1);
			flag2.store(1, memory_order_release);
		}
		rflag1 = flag1.load(memory_order_acquire);
	}
	// } // end while
	assume(rflag1 < 1);

	// critical section
	_cc_x.store(1, memory_order_release);
	rx = _cc_x.load(memory_order_acquire);
	assert(rx>=1);

	turn.store(0, memory_order_release);
	flag2.store(0, memory_order_release);
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
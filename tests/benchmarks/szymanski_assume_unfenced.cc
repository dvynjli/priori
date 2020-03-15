#include "../verify.h"

#define LOOP 3
#define OUTER_LOOP 2
#define LB LOOP*OUTER_LOOP

// shared variables
atomic<int> flag1; 
atomic<int> flag2;

atomic<int> _cc_x;

atomic<int> __fence_var;

void *p1(void *arg)
{
	flag1.store(1, memory_order_release);
	int rflag2 = flag2.load(memory_order_acquire);
	assume(rflag2 < 3);
	flag1.store(3, memory_order_release);
	rflag2 = flag2.load(memory_order_acquire);
	if(rflag2 == 1) {
		flag1.store(2, memory_order_release);
		rflag2 = flag2.load(memory_order_acquire);
		assume(rflag2 == 4);
	}
	flag1.store(4, memory_order_release);
	rflag2 = flag2.load(memory_order_acquire);
	assume(rflag2 < 2);

	// Critical Section
	_cc_x.store(0, memory_order_release);
	int rx = _cc_x.load(memory_order_acquire);
	assert(rx<=0);

	rflag2 = flag2.load(memory_order_acquire);
	assume(2>rflag2 || rflag2 > 3);
	flag1.store(0, memory_order_release);

	return NULL;
}

void *p2(void *arg)
{
	flag2.store(1, memory_order_release);
	int rflag1 = flag1.load(memory_order_acquire);
	assume(rflag1 < 3);
	flag2.store(3, memory_order_release);
	rflag1 = flag1.load(memory_order_acquire);
	if (rflag1 == 1) {
		flag2.store(2, memory_order_release);
		rflag1 = flag1.load(memory_order_acquire);
		assume(rflag1 == 4);
	}
	flag2.store(4, memory_order_release);
	rflag1 = flag1.load(memory_order_acquire);
	assume(rflag1 < 2);

	// Critical section
	_cc_x.store(1, memory_order_release);
	int rx = _cc_x.load(memory_order_acquire);
	assert(rx>=1);

	rflag1 = flag1.load(memory_order_acquire);
	assume(2 > rflag1 || rflag1 > 3);
	flag2.store(0, memory_order_release);

	return NULL;
}

int main()
{
	pthread_t a, b;

  pthread_create(&a, NULL, p1, NULL);
  pthread_create(&b, NULL, p2, NULL);

  pthread_join(a, NULL);
  pthread_join(b, NULL);

 return 0;
}

#include "../verify.h"

atomic<int> c1,c2;
atomic<int> n1,n2;
atomic<int> _cc_x;

void *t0(void *arg) {
	c1.store(1, memory_order_release);
	// __fence_var.fetch_add(0, memory_order_acq_rel);
	int rn2 = n2.load(memory_order_acquire);
	int rn1 = rn2+1;
	n1.store(rn1, memory_order_release);
	// __fence_var.fetch_add(0, memory_order_acq_rel);
	c1.store(0, memory_order_release);
	// __fence_var.fetch_add(0, memory_order_acq_rel);
	int rc2 = c2.load(memory_order_acquire);
	assume(rc2 == 0);
	// __fence_var.fetch_add(0, memory_order_acq_rel);
	rn2 = n2.load(memory_order_acquire);
	assume(rn2 == 0 || rn1 < rn2);

	// critical section
	_cc_x.store(0, memory_order_release);
	int rx = _cc_x.load(memory_order_acquire);
	assert(rx<=0);

	// __fence_var.fetch_add(0, memory_order_acq_rel);
	n1.store(0, memory_order_release);
	return NULL;
} 

void *t1(void *arg) {
	c2.store(1, memory_order_release);
	// __fence_var.fetch_add(0, memory_order_acq_rel);
	int rn1 = n1.load(memory_order_acquire);
	int rn2 = rn1+1;
	n2.store(rn2, memory_order_release);
	// __fence_var.fetch_add(0, memory_order_acq_rel);
	c2.store(0, memory_order_release);
	// __fence_var.fetch_add(0, memory_order_acq_rel);
	int rc1 = c1.load(memory_order_acquire);
	assume(rc1 == 0);
	// __fence_var.fetch_add(0, memory_order_acq_rel);
	rn1 = n1.load(memory_order_acquire);
	assume(rn1 == 0 || rn2 <= rn1);

	// critical section
	_cc_x.store(1, memory_order_release);
	int rx = _cc_x.load(memory_order_acquire);
	assert(rx>=1);
	
	// __fence_var.fetch_add(0, memory_order_acq_rel);
	n2.store(0,memory_order_release);
	return NULL;
}

int main()
{
 pthread_t a, b;

 pthread_create(&a, NULL, t0, NULL);
 pthread_create(&b, NULL, t1, NULL);

 pthread_join(a, NULL);
 pthread_join(b, NULL);

 return 0;
}

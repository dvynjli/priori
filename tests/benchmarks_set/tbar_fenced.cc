#include "../verify.h"

atomic<int> childnotready;
atomic<int> parentsense;
atomic<int> tdiff0, tdiff1;
atomic<int> __fence_var;

void *t0(void *arg) {
	int rsense = 1;

	tdiff0.store(1, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	int rchildnotready = childnotready.load(memory_order_acquire);
	assume(rchildnotready==0);
	childnotready.store(1, memory_order_release);
	parentsense.store(rsense, memory_order_release);
	rsense = 1 - rsense;
	__fence_var.fetch_add(0, memory_order_acq_rel);
	int rtdiff1 = tdiff1.load(memory_order_acquire);
	assert(rtdiff1!=0);
	return NULL;
}

void *t1(void *arg) {
	int rsense = 1;

	tdiff1.store(1, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	childnotready.store(0, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	int rparentsense = parentsense.load(memory_order_acquire);
	assume(rparentsense==rsense);
	rsense = 1 - rsense;
	__fence_var.fetch_add(0, memory_order_acq_rel);
	int rtdiff0 = tdiff0.load(memory_order_acquire);
	assert(rtdiff0!=0);
	return NULL;
}

int main()
{
	pthread_t a, b;

	//init
	childnotready.store(1, memory_order_release);

	pthread_create(&a, NULL, t0, NULL);
	pthread_create(&b, NULL, t1, NULL);

	pthread_join(a, NULL);
	pthread_join(b, NULL);

	return 0;
}

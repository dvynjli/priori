#include "../verify.h"

atomic<int> flag0;
atomic<int> flag1;
atomic<int> turn;

atomic_int var; // to avoid race

void *p0(void *arg)
{
	flag0.store(1, memory_order_release);
	// __fence_var.fetch_add(0, memory_order_acq_rel);
	turn.store(1, memory_order_release);
	// __fence_var.fetch_add(0, memory_order_acq_rel);
	int t1 = flag1.load(memory_order_acquire);
	int t2 = turn.load(memory_order_acquire);

	assume(t1!=1 || t2!=1);

	var.store(0, memory_order_release);
	int tmp = var.load(memory_order_acquire);
    assert(tmp==0);

	flag0.store(0, memory_order_release);

	return NULL;
}

void *p1(void *arg)
{
	flag1.store(1, memory_order_release);
	// __fence_var.fetch_add(0, memory_order_acq_rel);
	turn.store(0, memory_order_release);
	// __fence_var.fetch_add(0, memory_order_acq_rel);
	int t1 = flag0.load(memory_order_acquire);
	int t2 = turn.load(memory_order_acquire);

	assume(t1!=1 || t2==0);

	var.store(1, memory_order_release);
	int tmp = var.load(memory_order_acquire);
    assert(tmp==1);

	flag1.store(0, memory_order_release);

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

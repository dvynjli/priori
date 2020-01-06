#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x,y,z;
atomic<int> *p;

void* fun1(void * arg){
	// y.store(1, memory_order_release);
	// x.store(1,memory_order_acq_rel);
	return NULL;
}

void* fun2(void * arg){
	p = &y;
	if (x.load(memory_order_acquire)) p = &z;
	// else p = &z;
	p->store(1, memory_order_release);
	return NULL;
}

void* fun3(void * arg){
	atomic_int *a;
	int tmp1 = x.load(memory_order_acquire);
	if (tmp1) a = &y;
	else a = &z;
	// if (tmp1 == 2) {
		
		// testcase for alias analysis
		// tmp1==2 => tmp==1 should pass
		// assert(tmp2==1);
	int tmp2 = a->load(memory_order_acquire);
	assert(tmp2 == 1);
	// }
	return NULL;
}




int main () {
	pthread_t t1,t2,t3;
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	pthread_create(&t3, NULL, fun3, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);
	return 0;
}

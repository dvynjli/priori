// Testcase to check feasible interferences
// An interf a -> b, a-> c is not feasible

#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x;

void* fun1(void * arg){
	x.fetch_add(1, memory_order_acq_rel);				// inst a
	return NULL;
}

void* fun2(void * arg){
	x.fetch_add(1, memory_order_acq_rel);				// inst b
	int tmp = x.fetch_add(1, memory_order_acq_rel);		// inst c
	// assertion should pass
	assert(tmp<=3);
	return NULL;
}


int main () {
	pthread_t t1,t2;
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	return 0;
}

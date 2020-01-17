#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x,y;

void* fun1(void * arg){
	y.store(1, memory_order_release);
	x.fetch_add(1,memory_order_acq_rel);
	return NULL;
}

void* fun2(void * arg){
	int tmp1 = x.load(memory_order_acq_rel);
	int tmp2 = y.load(memory_order_acquire);
	// testcase for rmw synchronization with load
	// tmp1==1 => tmp==1 should pass
	assert(tmp1!=1 || tmp2==1);
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

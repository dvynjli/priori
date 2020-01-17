#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x;

void* fun1(void * arg){
	x.fetch_add(3,memory_order_acq_rel);
	return NULL;
}


int main () {
	pthread_t t1;
	x.store(2, memory_order_release);
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_join(t1, NULL);
	int tmp = x.load(memory_order_acquire);
	// testcase for rmw and pthread join
	// tmp == 5 should pass
	assert(tmp==5);
	return 0;
}

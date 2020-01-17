#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x;

void* fun1(void * arg){
	x.store(1, memory_order_release);
	int tmp1 = x.load(memory_order_acquire);
	// testcase to check if thread create is working correctly
	// tmp1 != 0 should pass
	assert(tmp1!=0);
	return NULL;
}


int main () {
	pthread_t t1;
	x.store(1, memory_order_release);
	pthread_create(&t1, NULL, fun1, NULL);
	// x.store(2, memory_order_release);
	pthread_join(t1, NULL);
	return 0;
}

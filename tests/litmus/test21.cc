#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x;

void* fun1(void * arg){
	x.store(1,memory_order_release);
	// x.store(2, memory_order_release);
	return NULL;
}

void* fun2(void * arg){
	int tmp1 = x.load(memory_order_acquire);
	if (tmp1==1) {
		int tmp2=x.load(memory_order_acquire);
		// testcase to check if branching is handled proerly
		// check the possible interfs
		// assert x!=0 should pass
		assert(tmp2!=0);
	}
	return NULL;
}


int main () {
	x.store(0, memory_order_release);

	pthread_t t1,t2;
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	return 0;
}

#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x,y;

void* fun1(void * arg){
	x.store(1, memory_order_release);
	y.store(1, memory_order_release);
	return NULL;
}

void* fun2(void * arg){
	x.store(2, memory_order_release);
	y.store(2, memory_order_release);
	return NULL;
}

void* fun3(void * arg){
	int tmp1 = x.load(memory_order_acquire);
	int tmp2 = y.load(memory_order_acquire);
	int tmp3 = tmp1 && tmp2;
	if (tmp3) {
		int tmp4 = x.load(memory_order_acquire);
		// Testcase checks that an older value can not be read
		// Assertion should pass
		assert(tmp4 != 0);
	}
	return NULL;
}


int main () {
	x.store(0, memory_order_release);
	y.store(0, memory_order_release);

	pthread_t t1,t2,t3;
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	pthread_create(&t3, NULL, fun3, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);
	return 0;
}

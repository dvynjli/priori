// to test PO

#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x,y;

void* fun1(void * arg){
	// x.store(5, memory_order_relaxed);
	x.store(1, memory_order_release);
	x.store(2, memory_order_release);
	return NULL;
}

void* fun2(void * arg){
	y.store(1, memory_order_release);
	y.store(2, memory_order_release);
	return NULL;
}

int main () {
	pthread_t t1,t2;
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	
	int tx = x.load(memory_order_acquire);
	int ty = y.load(memory_order_acquire);
	// x and y can be only 2 after the join. assertion should pass
	assert(tx==2 && ty==2);

	return 0;
}

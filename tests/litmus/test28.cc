// Testcase to check interfence with instructions after thread join

#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x;

void* fun1(void * arg){
	x.store(1, memory_order_release);
	return NULL;
}

void* fun2(void * arg){
	x.store(1, memory_order_release);
	int tmp = x.load(memory_order_acquire);
	// assertion should pass
	assert(tmp!=10);
	return NULL;
}


int main () {
	pthread_t t1,t2;
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	x.store(10,memory_order_release);
	return 0;
}

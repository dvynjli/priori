#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x;

void* fun1(void * arg){
	x.load(memory_order_acquire);
	x.store(1, memory_order_release);
	x.store(2, memory_order_release);
	x.load(memory_order_acquire);
	return NULL;
}

void* fun2(void * arg){
	x.load(memory_order_acquire);
	x.store(3, memory_order_release);
	x.store(4, memory_order_release);
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

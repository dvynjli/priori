#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x,y;

void* fun2(void * arg){
	x.store(10, memory_order_relaxed);
	// int a = x.load(memory_order_relaxed);
	y.store(20, memory_order_release);
	return NULL;
}

void* fun1(void * arg){
	x.store(50, memory_order_relaxed);
	y.load(memory_order_acquire);
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

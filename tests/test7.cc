#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x,y;

void* fun1(void * arg){
	y.store(10, memory_order_relaxed);
	// int a = x.load(memory_order_relaxed);
	x.store(20, memory_order_release);
	return NULL;
}

void* fun2(void * arg){
	x.load(memory_order_acquire);
	x.store(50, memory_order_release);
	return NULL;
}

void* fun3(void * arg){
	int a = x.load(memory_order_acquire);
	int b = y.load(memory_order_relaxed);
	// (x==50) ==> (y!=0) should fail
	assert(a!=50 || b!=0);
	return NULL;
}

int main () {
	pthread_t t1,t2,t3;
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	pthread_create(&t3, NULL, fun3, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);
	
	return 0;
}

#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x,y,done1,done2;

void* fun1(void * arg){
	x.store(1, memory_order_release);
	return NULL;
}

void* fun2(void * arg){
	y.store(1, memory_order_release);
	return NULL;
}

void* fun3(void * arg){
	int a = x.load(memory_order_acquire);
	int b = y.load(memory_order_acquire);
	if (a==1 && b==0) {
		done1.store(1, memory_order_relaxed);
	}
	return NULL;
}

void* fun4(void * arg){
	int a = y.load(memory_order_acquire);
	int b = x.load(memory_order_acquire);
	if (a==1 && b==0) {
		done2.store(1, memory_order_relaxed);
	}
	return NULL;
}

void *property (void *arg) {
	int a = done1.load(memory_order_relaxed);
	int b = done2.load(memory_order_relaxed);
	// no total ordering on writes of x and y. assertion should fail.
	assert(a!=1 || b!=1);
	return NULL;
}

int main () {
	pthread_t t1,t2,t3,t4, t5;
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	pthread_create(&t3, NULL, fun3, NULL);
	pthread_create(&t4, NULL, fun4, NULL);
	pthread_create(&t5, NULL, property, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);
	pthread_join(t4, NULL);
	pthread_join(t5, NULL);
	// int a = done1.load(memory_order_relaxed);
	// int b = done2.load(memory_order_relaxed);
	// no total ordering on writes of x and y. assertion should fail.
	// assert(a!=1 || b!=1);
	
	return 0;
}

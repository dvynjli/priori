#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x, y;
int b;
float c;

void* fun2(void * arg){
	// x.compare_exchange_strong(a, b, memory_order_relaxed);
	// atomic_fetch_add_explicit(&x, 1, memory_order_acq_rel);
	x.store(10, memory_order_relaxed);
	y.load(memory_order_seq_cst);
	x.store(20, memory_order_seq_cst);
	// y.load(memory_order_relaxed);
	return NULL;
}

void* fun1(void * arg){
	pthread_t t1;
	pthread_create(&t1, NULL, fun2, NULL);
	int a = 10;
	b = 5;
	int c = x.load(memory_order_acquire);
	y.store(a+c, memory_order_relaxed);
	c = y.load(memory_order_acq_rel);
	x.store(c+b, memory_order_relaxed);
	pthread_join(t1, NULL);
	return NULL;
}

void* fun3(void* arg){
	y.store(100, memory_order_relaxed);
	return NULL;
}

int main () {
	pthread_t t1,t2;
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun3, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	
	return 0;
}

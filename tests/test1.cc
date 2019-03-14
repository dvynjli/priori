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
	x.store(y.load(memory_order_relaxed)+b, memory_order_relaxed);
	int a = 10;
	b = 5;
	y.store(a+x.load(memory_order_relaxed), memory_order_relaxed);
	// y.load(memory_order_relaxed);
	return NULL;
}

void* fun1(void * arg){
	pthread_t t1;
	pthread_create(&t1, NULL, fun2, NULL);
	x.store(b, memory_order_relaxed);
	// y = b + 1;
	// x = y + 1;
	pthread_join(t1, NULL);
	return NULL;
}

int main () {
	pthread_t t1,t2;
	pthread_create(&t1, NULL, fun1, NULL);
	// pthread_create(&t2, NULL, fun1, NULL);
	pthread_join(t1, NULL);
	// pthread_join(t2, NULL);
	
	return 0;
}

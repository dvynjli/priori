#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x,y,a,b;

void* fun1(void * arg){
	int tmp = x.load(memory_order_acquire);
	if (tmp) a.store(tmp, memory_order_release);
	y.store(1, memory_order_release);
	return NULL;
}

void* fun2(void * arg){
	int tmp = y.load(memory_order_acquire);
	if (tmp) b.store(tmp, memory_order_release);
	x.store(1, memory_order_release);
	return NULL;
}


int main () {
	x.store(0, memory_order_release);
	y.store(0, memory_order_release);
	a.store(0, memory_order_release);
	b.store(0, memory_order_release);

	pthread_t t1,t2;
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	int tmp1 = a.load(memory_order_acquire);
	int tmp2 = b.load(memory_order_acquire);
	// both a and b can't be 1
	// a==1 => b!=1 should pass
	assert(tmp1!=1 || tmp2!=1);
	return 0;
}

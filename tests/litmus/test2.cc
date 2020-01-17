#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x,y,z;

void* fun1(void * arg){
	int tmp = x.load(memory_order_acquire);
	z.store(tmp, memory_order_release);
	y.store(2, memory_order_release);
	x.store(2, memory_order_release);
	return NULL;
}

void* fun2(void * arg){
	y.store(1, memory_order_release);
	x.store(1, memory_order_release);
	int a = x.load(memory_order_acquire);
	int b = y.load(memory_order_acquire);
	int c = z.load(memory_order_acquire);
	// (x==2 && z==1) => y=2 should hold
	assert((a!=2 || c!=1) || b==2);
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

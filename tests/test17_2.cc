#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x,y,z;

void* fun1(void * arg){
	y.store(1, memory_order_relaxed);
	x.store(1, memory_order_release);
	return NULL;
}

void* fun2(void * arg){
	int tmp = x.load(memory_order_acquire);
	z.store(tmp, memory_order_relaxed);
	y.store(2, memory_order_relaxed);
	x.store(2, memory_order_release);
	return NULL;
}

void* fun3(void * arg){
	int a = x.load(memory_order_acquire);
	int b = y.load(memory_order_relaxed);
	int c = z.load(memory_order_relaxed);
	// (x==2 && z==1) ==> (y!=1) should hold
	assert((a!=2 || c!=1) || b!=1);
	// (x==2 ==> y!=0) should hold
	assert(a!=2 || b!=0);
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

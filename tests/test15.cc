#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x, y;

void* fun1(void * arg){
	x.store(1, memory_order_release);
	return NULL;
}

void* fun2(void * arg){
	x.load(memory_order_acquire);
	y.store(1, memory_order_relaxed);
	x.store(2, memory_order_release);
	return NULL;
}

void* fun3(void * arg){
	int tmp1 = x.load(memory_order_acquire);
	int tmp2 = y.load(memory_order_relaxed);
	// x==2 ==> y!=0 should hold
	assert(tmp1!=2 || tmp2!=0);
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
	
	// (a==2 && b==3) ==> (c!=1) should not hold
	return 0;
}

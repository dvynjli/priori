#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x;
atomic<int> a,b,c;

void* fun1(void * arg){
	x.store(1, memory_order_relaxed);
	x.store(2, memory_order_relaxed);
	return NULL;
}

void* fun2(void * arg){
	int tmp = x.load(memory_order_acquire);
	a.store(tmp, memory_order_relaxed);
	x.store(3, memory_order_relaxed);
	return NULL;
}

void* fun3(void * arg){
	int tmp1 = x.load(memory_order_relaxed);
	b.store(tmp1, memory_order_relaxed);
	int tmp2 = x.load(memory_order_relaxed);
	c.store(tmp2, memory_order_relaxed);
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
	
	// (a==2 && b==3) ==> (c!=1) should fail
	int tmp1 = a.load(memory_order_relaxed);
	int tmp2 = b.load(memory_order_relaxed);
	int tmp3 = c.load(memory_order_relaxed);
	assert(tmp1!=2 || tmp2!=3 || tmp3!=1);
	return 0;
}

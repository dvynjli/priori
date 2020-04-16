#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x,y,a,b;

void* fun1(void * arg){
	int tmp = x.load(memory_order_acquire);
	x.store(tmp+1, memory_order_release);
	return NULL;
}

void* fun2(void * arg){
	int tmp = x.load(memory_order_acquire);
	x.store(tmp+1, memory_order_release);
	return NULL;
}


int main () {
	pthread_t t1,t2;
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	int tmp1 = x.load(memory_order_acquire);
	// both a and b can't be 1
	// x can be at max 2 should pass
	assert(tmp1 <= 2 );
	return 0;
}

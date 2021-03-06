#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x,y;

void* fun1(void * arg){
	x.store(1, memory_order_release);
	y.store(1, memory_order_release);
	return NULL;
}

void* fun2(void * arg){
	int tmp1 = y.load(memory_order_acquire);
	int tmp2 = x.load(memory_order_acquire);
	// tmp2 == 1 => tmp1 == 1 should fail
	assert((tmp2!=1) || (tmp1==1));
	return NULL;
}


int main () {
	pthread_t t1,t2,t3;
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	return 0;
}

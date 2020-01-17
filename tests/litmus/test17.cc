// (2+2W) from Lahav POPL 16

#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x,y;

void* fun1(void * arg){
	x.store(1,memory_order_release);
	y.store(2, memory_order_release);
	return NULL;
}

void* fun2(void * arg){
	y.store(1, memory_order_release);
	x.store(2, memory_order_release);
	return NULL;
}


int main () {
	pthread_t t1,t2;
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	int tmp1 = x.load(memory_order_acquire);
	int tmp2 = y.load(memory_order_acquire);
	// x and y can read value 1 here
	// x != 1 should fail
	assert(tmp1!=1);
	return 0;
}

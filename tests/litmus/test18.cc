#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x,y;

void* fun1(void * arg){
	x.store(1,memory_order_release);
	y.store(1, memory_order_release);
	return NULL;
}

void* fun2(void * arg){
	if (y.load(memory_order_acquire)) 	x.store(5, memory_order_release);
	else x.store(3, memory_order_release);
	y.store(2, memory_order_release);
	return NULL;
}


int main () {
	pthread_t t1,t2;
	x.store(0, memory_order_release);
	y.store(0, memory_order_release);
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	int tmp1 = y.load(memory_order_acquire);
	int tmp2 = x.load(memory_order_acquire);
	// testcase to check if mergering of branches is handled properly.
	// y==2 => (x==2 || x==3) should pass
	assert(tmp1!=2 || tmp2==5 || tmp2==3);
	return 0;
}

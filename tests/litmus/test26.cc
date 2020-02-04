#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x,y;
atomic<int> done1, done2;

void* fun1(void * arg){
	int tmp = x.load(memory_order_acquire);
	if (tmp==1) {
		done1.store(1, memory_order_release);
	}
	y.store(1, memory_order_release);
	return NULL;
}

void* fun2(void * arg){
	int tmp = y.load(memory_order_acquire);
	if (tmp==1) {
		done2.store(memory_order_release);
	}
	x.store(1, memory_order_release);
	return NULL;
}


int main () {
	pthread_t t1,t2;
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	int tmp1 = done1.load(memory_order_acquire);
	int tmp2 = done2.load(memory_order_acquire);
	// testcase if x reads 1, y can't read 1 and vice versa.
	// done1 == 1 => done2 != 1 should pass
	if (tmp1==1)
		assert(tmp2!=1);
	return 0;
}

// Testcase to check if the tool can handle array

#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> arr[2], x;

void* fun1(void * arg){
	x.store(1, memory_order_release);
	// x.store(2, memory_order_release);
	return NULL;
}

void* fun2(void * arg){
	int tmp = x.load(memory_order_acquire);
	// assertion should pass
	arr[tmp].store(1, memory_order_release);
	return NULL;
}


int main () {
	pthread_t t1,t2;
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	int tmp1 = arr[0].load(memory_order_acquire);
	int tmp2 = arr[1].load(memory_order_acquire);
	// arr[0]+arr[1] = 1 should pass
	assert(tmp1+tmp2==1);
	return 0;
}

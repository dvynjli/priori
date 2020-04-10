#include "../verify.h"

atomic<int> x,y;

void* fun1(void * arg){
	x.store(1, memory_order_release);
	return NULL;
}

void* fun2(void * arg){
	int rx = x.load(memory_order_acquire);
	y.store(rx, memory_order_release);
	return NULL;
}

void* fun3(void * arg){
	int rx = x.load(memory_order_acquire);
	int ry = y.load(memory_order_acquire);
	// (x==2 && z==1) => y=2 should hold
	assert(rx!=1 || ry!=1);
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

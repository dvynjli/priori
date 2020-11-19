#include "../verify.h"

atomic<int> x,done1,done2;

void* fun1(void * arg){
	x.store(1, memory_order_release);
	return NULL;
}

void* fun2(void * arg){
	x.store(3, memory_order_release);
	return NULL;
}

void* fun3(void * arg){
	int a = x.load(memory_order_acquire);
	int b = x.load(memory_order_acquire);
	if (a==1 && b==3) {
		done1.store(1, memory_order_relaxed);
	}
	return NULL;
}

void* fun4(void * arg){
	int a = x.load(memory_order_acquire);
	int b = x.load(memory_order_acquire);
	if (a==3 && b==1) {
		done2.store(1, memory_order_relaxed);
	}
	return NULL;
}

int main () {
	x.store(2, memory_order_release);
	done1.store(0, memory_order_release);
	done2.store(0, memory_order_release);

	pthread_t t1,t2,t3,t4;
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	pthread_create(&t3, NULL, fun3, NULL);
	pthread_create(&t4, NULL, fun4, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);
	pthread_join(t4, NULL);
	int a = done1.load(memory_order_relaxed);
	int b = done2.load(memory_order_relaxed);
	// Both writes of x should have total order. assertion should pass.
	assert(a!=1 || b!=1);
	
	return 0;
}

#include "../verify.h"

atomic<int> x;

void* fun1(void * arg){
	int rx1 = x.load(ACQ);
    // if (rx1)
	x.store(1, REL);
    int rx2 = x.load(ACQ);
    int rx3 = x.load(ACQ);
    assume (rx1 !=1 && (rx2==1 && rx3==1));
	// assume condition might be true
	// Assertion should fail
	// assert(1);
    return NULL;
}

void* fun2(void * arg){
	x.store(2, memory_order_release);
	return NULL;
}

/*
void* fun3(void * arg){
	int a = x.load(memory_order_acquire);
	int b = x.load(memory_order_acquire);
	if (a==1 && b==2) {
		done1.store(1, memory_order_relaxed);
	}
	return NULL;
}

void* fun4(void * arg){
	int a = x.load(memory_order_acquire);
	int b = x.load(memory_order_acquire);
	if (a==2 && b==1) {
		done2.store(1, memory_order_relaxed);
	}
	return NULL;
} */

int main () {
	pthread_t t1,t2,t3,t4;
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	// pthread_create(&t3, NULL, fun3, NULL);
	// pthread_create(&t4, NULL, fun4, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	// pthread_join(t3, NULL);
	// pthread_join(t4, NULL);
	// int a = done1.load(memory_order_relaxed);
	// int b = done2.load(memory_order_relaxed);
	// // Both writes of x should have total order. assertion should pass.
	// assert(a!=1 || b!=1);
	
	return 0;
}

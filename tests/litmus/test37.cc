#include "../verify.h"
#include <mutex>

atomic<int> x, y;

void* fun1(void * arg){
	x.store(1,memory_order_release);
	x.store(2,memory_order_release);
    return NULL;
}

void* fun2(void * arg){
	int rx=x.load(memory_order_acquire);
	switch(rx) {
		case 1: y.store(10, memory_order_release); break;
		case 2: y.store(20, memory_order_release); break;
		case 3: y.store(30, memory_order_release); break;
	}
    return NULL;
}

int main () {
	pthread_t t1,t2,t3,t4;
    // x.store(2,REL);
    // y.store(2,REL);
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	
	return 0;
}

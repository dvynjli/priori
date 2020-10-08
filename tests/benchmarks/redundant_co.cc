#include "../verify.h"

atomic<int> x;

void* t1(void * arg){
	x.store(1,memory_order_release);
	x.store(1,memory_order_release);
	x.store(1,memory_order_release);
	x.store(1,memory_order_release);
	x.store(1,memory_order_release);
	
	x.store(1,memory_order_release);
	x.store(1,memory_order_release);
	x.store(1,memory_order_release);
	x.store(1,memory_order_release);
	x.store(1,memory_order_release);

	x.store(1,memory_order_release);
	x.store(1,memory_order_release);
	x.store(1,memory_order_release);
	x.store(1,memory_order_release);
	x.store(1,memory_order_release);

	x.store(1,memory_order_release);
	x.store(1,memory_order_release);
	x.store(1,memory_order_release);
	x.store(1,memory_order_release);
	x.store(1,memory_order_release);

	return NULL;
}

void* t2(void * arg){
	x.store(2,memory_order_release);
	x.store(2,memory_order_release);
	x.store(2,memory_order_release);
	x.store(2,memory_order_release);
	x.store(2,memory_order_release);

	x.store(2,memory_order_release);
	x.store(2,memory_order_release);
	x.store(2,memory_order_release);
	x.store(2,memory_order_release);
	x.store(2,memory_order_release);

	x.store(2,memory_order_release);
	x.store(2,memory_order_release);
	x.store(2,memory_order_release);
	x.store(2,memory_order_release);
	x.store(2,memory_order_release);

	x.store(2,memory_order_release);
	x.store(2,memory_order_release);
	x.store(2,memory_order_release);
	x.store(2,memory_order_release);
	x.store(2,memory_order_release);

	return NULL;
}

void* t3(void * arg){
	x.load(memory_order_acquire);
	x.load(memory_order_acquire);
	return NULL;
}

int main () {
	pthread_t th1,th2,th3;

	pthread_create(&th1, NULL, t1, NULL);
	pthread_create(&th2, NULL, t2, NULL);
	pthread_create(&th3, NULL, t3, NULL);

	pthread_join(th1, NULL);
	pthread_join(th2, NULL);
	pthread_join(th3, NULL);
	
	return 0;
}

#include "../verify.h"
#include <mutex>

// mutex *mplock;
mutex mlock;
atomic<int> x;


void* fun1(void * arg){
	mlock.lock();
	x=1;
	mlock.unlock();
    return NULL;
}

void* fun2(void * arg){
	mlock.lock();
	int rx=x;
	mlock.unlock();
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

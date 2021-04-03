#include "../verify.h"
#include <mutex>

atomic<int> x;

void* fun1(void * arg){
	x.store(1, REL);
    return NULL;
}

void* fun2(void * arg){
	x.fetch_add(1, ACQREL);
    return NULL;
}

void* fun3(void *arg) {
	x.store(3, REL);
	return NULL;
}

void* fun4(void *arg) {
	int rx1 = x.load(ACQ);
	int rx2 = x.load(ACQ);
	int rx3 = x.load(ACQ);
	assert(!(rx1==1 && rx2==3 && rx3==2));
	return NULL;
}

int main () {
	pthread_t t1,t2,t3,t4;
    x.store(0,REL);
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	pthread_create(&t3, NULL, fun3, NULL);
	pthread_create(&t4, NULL, fun4, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);
	pthread_join(t4, NULL);
	
	return 0;
}

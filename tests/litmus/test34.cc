#include "../verify.h"

atomic<int> x,y,r1,r2;

void* fun1(void * arg){
	x.store(1, REL);
    int tmp = y.load(ACQ);
    r1.store(tmp,REL);
    return NULL;
}

void* fun2(void * arg){
	y.store(1, REL);
	int tmp = x.load(ACQ);
    r2.store(tmp,REL);
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
	int a = r1.load(ACQ);
    int b = r2.load(ACQ);
    // assert(a!=2 || b!=2);
	
	return 0;
}

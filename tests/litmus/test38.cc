#include "../verify.h"
#include <mutex>

atomic<int> x, y;
atomic<int> var, fence;

void* fun1(void * arg){
	y.store(1, REL);
	fence.fetch_add(0, ACQREL);
	int tmp1 = x.load(ACQ);
	int done = 1;
	if (tmp1 == 1) {
		y.store(0, REL);
		done = 0;
	}
	assume(done==1);
	var.store(0, REL);
	int tmp2 = var.load(ACQ);
	assert(tmp2 == 0);
    return NULL;
}

void* fun2(void * arg){
	x.store(1,REL);
	fence.fetch_add(0, ACQREL);
	int tmp1 = y.load(ACQ);
	int done = 1;
	if (tmp1 == 1) {
		x.store(0, REL);
		done = 0;
	}
	assume(done==1);
	var.store(1, REL);
	int tmp2 = var.load(ACQ);
	assert(tmp2 == 1);
    return NULL;
}

int main () {
	pthread_t t1,t2,t3,t4;
    x.store(0,REL);
    y.store(0,REL);
    fence.store(0,REL);
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	
	// int rx = x.load(ACQ);
	// int ry = y.load(ACQ);
	// assert(rx!=1 || ry!=1);

	return 0;
}

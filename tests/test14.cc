#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x,y;
atomic<int> *p;

void* fun1(void * arg){
	y.store(1, memory_order_release);
	x.store(1,memory_order_acq_rel);
	return NULL;
}

void* fun2(void * arg){
	if (x.load(memory_order_acquire)) {
		p = &y;
		x.store(2, memory_order_release);
	}
	return NULL;
}

void* fun3(void * arg){
	int tmp1 = x.load(memory_order_acquire);
	// cout << tmp1 << endl;
	if (tmp1 == 2) {
		int tmp2 = p->load(memory_order_acquire);
		// testcase for alias analysis
		// tmp1==2 => tmp==1 should pass
		assert(tmp2==1);
	}
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

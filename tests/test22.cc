// thread partition version of (2+2W) from Lahav-POPL16
// Taming release-acquire consistency

#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x,y,z1,z2,a,b;

void* fun1(void * arg){
	x.store(1,memory_order_release);
	y.store(2,memory_order_release);
	z1.store(1,memory_order_release);
	int tmp1 = z2.load(memory_order_acquire);
	int tmp2 = x.load(memory_order_acquire);
	if (tmp1==1 && tmp2==1) {
		a.store(1, memory_order_release);
	}
	return NULL;
}

void* fun2(void * arg){
	y.store(1,memory_order_release);
	x.store(2,memory_order_release);
	z2.store(1,memory_order_release);
	int tmp1 = z1.load(memory_order_acquire);
	int tmp2 = y.load(memory_order_acquire);
	if (tmp1==1 && tmp2==1) {
		b.store(1, memory_order_release);
	}
	return NULL;
}


int main () {
	pthread_t t1,t2;
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	int tmp1 = a.load(memory_order_acquire);
	int tmp2 = b.load(memory_order_acquire);
	// assignment of 1 to a and b is same as completion of thread in original program
	// both a and b can be 1 in a single execution
	// assertion a==1 => b!=1 should fail
	assert(tmp1!=1 || tmp2!=1);
	return 0;
}

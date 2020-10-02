#include "../verify.h"

atomic<int> x,y,z, done1;

void* fun1(void * arg){
	x.store(1, REL);
	return NULL;
}

void* fun2(void * arg){
	int tmp1 = x.load(ACQ);
    if(tmp1) {
        x.store(2, REL);
        y.store(2, REL);
        int tmp2 = z.load(ACQ);
        if (tmp2==0) done1.store(1, REL);
    }
    return NULL;
}

void* fun3(void * arg){
	int tmp1 = z.load(ACQ);
	int tmp2 = y.load(ACQ);
    int tmp3 = x.load(ACQ);
    int tmp4 = done1.load(ACQ);
	if (tmp2 == 2) assert(tmp3==2);
    // if (tmp4 && tmp1) assert (tmp3!=0); 
    return NULL;
}

void* fun4(void * arg){
	z.store(REL);
	return NULL;
}

int main () {
	x.store(0, memory_order_release);
	y.store(0, memory_order_release);
	z.store(0, memory_order_release);
	done1.store(0, memory_order_release);

	pthread_t t1,t2,t3,t4;
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	pthread_create(&t3, NULL, fun3, NULL);
	pthread_create(&t4, NULL, fun4, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);
	pthread_join(t4, NULL);
	// int a = done1.load(memory_order_relaxed);
	// int b = done2.load(memory_order_relaxed);
	// Both writes of x should have total order. assertion should pass.
	// assert(a!=?1 || b!=1);
	
	return 0;
}

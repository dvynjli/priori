#include "../verify.h"

atomic<int> x;
atomic<int> n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15,n16;

void* t1(void * arg){
	int tmp = x.load(memory_order_acquire);
	n1.store(tmp, memory_order_release);
	return NULL;
}

void* t2(void * arg){
	int tmp = x.load(memory_order_acquire);
	n2.store(tmp, memory_order_release);
	return NULL;
}

void* t3(void * arg){
	int tmp = x.load(memory_order_acquire);
	n3.store(tmp, memory_order_release);
	return NULL;
}

void* t4(void * arg){
	int tmp = x.load(memory_order_acquire);
	n4.store(tmp, memory_order_release);
	return NULL;
}

void* t5(void * arg){
	int tmp = x.load(memory_order_acquire);
	n5.store(tmp, memory_order_release);
	return NULL;
}

void* t6(void * arg){
	int tmp = x.load(memory_order_acquire);
	n6.store(tmp, memory_order_release);
	return NULL;
}

void* t7(void * arg){
	int tmp = x.load(memory_order_acquire);
	n7.store(tmp, memory_order_release);
	return NULL;
}

void* t8(void * arg){
	int tmp = x.load(memory_order_acquire);
	n8.store(tmp, memory_order_release);
	return NULL;
}

void* t9(void * arg){
	int tmp = x.load(memory_order_acquire);
	n9.store(tmp, memory_order_release);
	return NULL;
}

void* t10(void * arg){
	int tmp = x.load(memory_order_acquire);
	n10.store(tmp, memory_order_release);
	return NULL;
}

void* t11(void * arg){
	int tmp = x.load(memory_order_acquire);
	n11.store(tmp, memory_order_release);
	return NULL;
}

void* t12(void * arg){
	int tmp = x.load(memory_order_acquire);
	n12.store(tmp, memory_order_release);
	return NULL;
}

void* t13(void * arg){
	int tmp = x.load(memory_order_acquire);
	n13.store(tmp, memory_order_release);
	return NULL;
}

void* t14(void * arg){
	int tmp = x.load(memory_order_acquire);
	n14.store(tmp, memory_order_release);
	return NULL;
}

void* t15(void * arg){
	int tmp = x.load(memory_order_acquire);
	n15.store(tmp, memory_order_release);
	return NULL;
}

void* t16(void * arg){
	int tmp = x.load(memory_order_acquire);
	n16.store(tmp, memory_order_release);
	return NULL;
}


void* writer(void * arg){
	x.store(1,memory_order_release);
	return NULL;
}

// void* property_checking(void * arg) {
//     int tmp1 = n1.load(memory_order_acquire);
//     int tmp2 = n2.load(memory_order_acquire);
//     int tmp3 = n3.load(memory_order_acquire);
//     assert(tmp1<=0 || tmp2<=0 || tmp3<=0);
//     return NULL;
// }

int main () {
	pthread_t th1,th2,th3,th4,th5,
				th6,th7,th8,th9,th10,
				th11,th12,th13,th14,th15,
				th16;
    pthread_t th_w; 
	// pthread_t property;
	pthread_create(&th1, NULL, t1, NULL);
	pthread_create(&th2, NULL, t2, NULL);
	pthread_create(&th3, NULL, t3, NULL);
	pthread_create(&th4, NULL, t4, NULL);
	pthread_create(&th5, NULL, t5, NULL);
	pthread_create(&th6, NULL, t6, NULL);
	pthread_create(&th7, NULL, t7, NULL);
	pthread_create(&th8, NULL, t8, NULL);
	pthread_create(&th9, NULL, t9, NULL);
	pthread_create(&th10, NULL, t10, NULL);
	pthread_create(&th11, NULL, t11, NULL);
	pthread_create(&th12, NULL, t12, NULL);
	pthread_create(&th13, NULL, t13, NULL);
	pthread_create(&th14, NULL, t14, NULL);
	pthread_create(&th15, NULL, t15, NULL);
	pthread_create(&th16, NULL, t16, NULL);
	
	pthread_create(&th_w, NULL, writer, NULL);
	//pthread_create(&property, NULL, property_checking, NULL);

	pthread_join(th1, NULL);
	pthread_join(th2, NULL);
	pthread_join(th3, NULL);
	pthread_join(th4, NULL);
	pthread_join(th5, NULL);
	pthread_join(th6, NULL);
	pthread_join(th7, NULL);
	pthread_join(th8, NULL);
	pthread_join(th9, NULL);
	pthread_join(th10, NULL);
	pthread_join(th11, NULL);
	pthread_join(th12, NULL);
	pthread_join(th13, NULL);
	pthread_join(th14, NULL);
	pthread_join(th15, NULL);
	pthread_join(th16, NULL);

	pthread_join(th_w, NULL);
	//pthread_join(property, NULL);

	int tmp1 = n1.load(memory_order_acquire);
    int tmp2 = n2.load(memory_order_acquire);
    int tmp3 = n3.load(memory_order_acquire);
    int tmp4 = n4.load(memory_order_acquire);
    int tmp5 = n5.load(memory_order_acquire);
    int tmp6 = n6.load(memory_order_acquire);
    int tmp7 = n7.load(memory_order_acquire);
    int tmp8 = n8.load(memory_order_acquire);
    int tmp9 = n9.load(memory_order_acquire);
    int tmp10 = n10.load(memory_order_acquire);
    int tmp11 = n11.load(memory_order_acquire);
    int tmp12 = n12.load(memory_order_acquire);
    int tmp13 = n13.load(memory_order_acquire);
    int tmp14 = n14.load(memory_order_acquire);
    int tmp15 = n15.load(memory_order_acquire);
    int tmp16 = n16.load(memory_order_acquire);
    assert(tmp1==0 || tmp2==0 || tmp3==0 || tmp4==0 || tmp5==0 
    	|| tmp6==0 || tmp7==0 || tmp8==0 || tmp9==0 || tmp10==0 
    	|| tmp11==0 || tmp12==0 || tmp13==0 || tmp14==0 || tmp15==0 
    	|| tmp16==0);
	
	return 0;
}


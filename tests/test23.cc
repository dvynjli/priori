#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x;

void* fun1(void * arg){
	int tmp1;
	cin >> tmp1;
	x.store(tmp1, memory_order_release);
	return NULL;
}

void* fun2(void * arg){
	int tmp2;
	scanf("%d", &tmp2);
	x.store(tmp2, memory_order_release);
	return NULL;
}


int main () {
	pthread_t t1,t2;
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	return 0;
}

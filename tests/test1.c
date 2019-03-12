#include <stdio.h>
#include <pthread.h>
#include <assert.h>

int x, y;

void* fun2(void * arg){
	x = y + 1;
	int a = 10;
	y = a+x;
	return NULL;
}

void* fun1(void * arg){
	pthread_t t1;
	pthread_create(&t1, NULL, fun2, NULL);
	y = 10;
	pthread_join(t1, NULL);
	return NULL;
}

int main () {
	pthread_t t1,t2;
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun1, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	
	return 0;
}

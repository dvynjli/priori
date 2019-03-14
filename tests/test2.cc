#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

int x,y;
float c;

void* fun2(void * arg){
	x = 10;
	y = x + 1;
	return NULL;
}

void* fun1(void * arg){
	pthread_t t1;
	pthread_create(&t1, NULL, fun2, NULL);
	int a = 10+20;
	int b = a*a;
	y = x + b;
	pthread_join(t1, NULL);
	return NULL;
}

int main () {
	pthread_t t1,t2;
	pthread_create(&t1, NULL, fun1, NULL);
	// pthread_create(&t2, NULL, fun1, NULL);
	pthread_join(t1, NULL);
	// pthread_join(t2, NULL);
	
	return 0;
}

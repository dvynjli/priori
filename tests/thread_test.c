#include <stdio.h>
#include <pthread.h>
#include <assert.h>

int x, y;
//pthread_mutex_t lock;

void* fun2(void * arg){
	//for (int i = 0; i < 3; i++){
		//pthread_mutex_lock(&lock);
	if (x < 10)
		x++;
	//assert(y == 0);
		//pthread_mutex_unlock(&lock);
	//}
	return NULL;
}

void* fun1(void * arg){
	//for (int i = 0; i < 3; i++){
		//pthread_mutex_lock(&lock);
	pthread_t t1;
	pthread_create(&t1, NULL, fun2, NULL);
	if (x < 10)
		x++;
		//pthread_mutex_unlock(&lock);
	//}
	return NULL;
}

int main () {
	pthread_t t1,t2;
	// if (pthread_mutex_init(&lock, NULL) != 0)
	// {
	// 	return 1;
	// }
	pthread_create(&t1, NULL, fun1, NULL);
	pthread_create(&t2, NULL, fun1, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	//pthread_mutex_destroy(&lock);
	
	return 0;
}

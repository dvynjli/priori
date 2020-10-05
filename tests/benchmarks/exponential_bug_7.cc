/* Copyright (C) 2018 Tuan Phong Ngo 
 * This benchmark is part of TRACER
 */

/* Example in Figure 2 in the PLDI 2015 paper: 
   https://dl.acm.org/citation.cfm?id=2737975
*/
#include "../verify.h"
#include <mutex>

using namespace std;

#define N 7

// shared variables
atomic<int> x, y;
mutex mlock;

void* thr1(void *arg)
{
	mlock.lock();
	x.store(1,memory_order_release);
	// for (int i=0; i<N; i++) {
	y.store(1,memory_order_release);
	y.store(1,memory_order_release);
	y.store(1,memory_order_release);
	y.store(1,memory_order_release);
	y.store(1,memory_order_release);
	y.store(1,memory_order_release);
	y.store(1,memory_order_release);
	// }
	mlock.unlock();

	mlock.lock();
	x.store(1,memory_order_release);
	// for (int i=0; i<N; i++) {
	y.store(1,memory_order_release);
	y.store(1,memory_order_release);
	y.store(1,memory_order_release);
	y.store(1,memory_order_release);
	y.store(1,memory_order_release);
	y.store(1,memory_order_release);
	y.store(1,memory_order_release);
	// }
	mlock.unlock();
	return NULL;
}

void* thr2(void *arg)
{
	int _y;

	mlock.lock();
	x.store(0,memory_order_release);
	mlock.unlock();
	
	int rx = x.load(memory_order_acquire);
	if (rx > 0) {
		_y = y.load(memory_order_acquire);
		y.store(_y+1, memory_order_release);
		x.store(2,memory_order_release);
	}

	mlock.lock();
	x.store(0,memory_order_release);
	mlock.lock();
	
	rx = x.load(memory_order_acquire);
	if (rx > 0) {
		_y = y.load(memory_order_acquire);
		y.store(_y+1, memory_order_release);
		x.store(2,memory_order_release);
	}
	return NULL;
}


void* thr3(void *arg)
{
	int rx = x.load(memory_order_acquire);
	if (rx > 1) {
		int ry = y.load(memory_order_acquire);
		if (ry == 3) {
			//MODEL_ASSERT(0);
			return NULL;
		} else {
			y.store(2, memory_order_release);
		}
	}

	rx = x.load(memory_order_acquire);
	if (rx > 1) {
		int ry = y.load(memory_order_acquire);
		if (ry == 3) {
			//MODEL_ASSERT(0);
			return NULL;
		} else {
			y.store(2, memory_order_release);
		}
	}
	return NULL;
}

int main()
{

	pthread_t t1, t2, t3;
	
	pthread_create(&t1, NULL, thr1, NULL);
	pthread_create(&t2, NULL, thr2, NULL);
	pthread_create(&t3, NULL, thr3, NULL);


	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);
	return 0;
}

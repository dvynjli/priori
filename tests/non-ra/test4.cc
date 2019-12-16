#include <iostream>
#include <thread>
#include <assert.h>
#include <atomic>

using namespace std;

atomic<int> x,y;

void fun2(){
	int b;
	x.store(10, memory_order_relaxed);
}

void fun1(){
	x.store(50, memory_order_relaxed);
	// x.store(15, memory_order_relaxed);
	y.load(memory_order_relaxed);
}

int main () {
	thread a(fun1);
	thread b(fun2);
	a.join(); b.join();
	
	return 0;
}

/* Copyright (C) 2018 Tuan Phong Ngo 
 * This benchmark is part of TRACER
 */

/* Adapted from: https://svn.sosy-lab.org/software/sv-benchmarks/trunk/c/pthread/fib_bench_false-unreach-call.c */

#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

#define NUM 5
#define CORRECT 144

// shared variables
atomic_int i, j;

void* t1(void *arg)
{
	int _i, _j;
	// for (int k=0; k<NUM; k++) { 			// loop k: begin
		// k=0
		_i = i.load(memory_order_acquire);
		_j = j.load(memory_order_acquire);
		i.store(_i+_j, memory_order_release);
		// k=1
		_i = i.load(memory_order_acquire);
		_j = j.load(memory_order_acquire);
		i.store(_i+_j, memory_order_release);
		// // k=2
		// _i = i.load(memory_order_acquire);
		// _j = j.load(memory_order_acquire);
		// i.store(_i+_j, memory_order_release);
		// // k=3
		// _i = i.load(memory_order_acquire);
		// _j = j.load(memory_order_acquire);
		// i.store(_i+_j, memory_order_release);
		// // k=4
		// _i = i.load(memory_order_acquire);
		// _j = j.load(memory_order_acquire);
		// i.store(_i+_j, memory_order_release);
	// }									// loop k: end
	return NULL;
}

void* t2(void *arg)
{
	int _i, _j;
	// for (int k=0; k<NUM; k++) {			// loop k: begin
		// k=0
		_i = i.load(memory_order_acquire);
		_j = j.load(memory_order_acquire);
		j.store(_i+_j, memory_order_release);
		// // k=1
		// _i = i.load(memory_order_acquire);
		// _j = j.load(memory_order_acquire);
		// j.store(_i+_j, memory_order_release);
		// // k=2
		// _i = i.load(memory_order_acquire);
		// _j = j.load(memory_order_acquire);
		// j.store(_i+_j, memory_order_release);
		// // k=3
		// _i = i.load(memory_order_acquire);
		// _j = j.load(memory_order_acquire);
		// j.store(_i+_j, memory_order_release);
		// // k=4
		// _i = i.load(memory_order_acquire);
		// _j = j.load(memory_order_acquire);
		// j.store(_i+_j, memory_order_release);
	// }									// loop k: end
	return NULL;
}

// static int fib(int n) {
// 	int cur = 1, prev = 0;
// 	while(n--) {
// 		int next = prev+cur;
// 		prev = cur;
// 		cur = next;
// 	}
// 	return prev;
// }

int main(int argc, char **argv)
{
	pthread_t a, b;

	i.store(1, memory_order_release);
	j.store(1, memory_order_release);

	pthread_create(&a, NULL, t1, NULL);
	pthread_create(&b, NULL, t2, NULL);

	pthread_join(a, NULL);
	pthread_join(b, NULL);

	// int correct = fib(2+2*NUM);
	int _i = i.load(memory_order_acquire);
	int _j = j.load(memory_order_acquire);

	if (_i > CORRECT || _j > CORRECT) {
		assert(0);
	}

	return 0;
}

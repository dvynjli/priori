/* Copyright (C) 2018 Tuan Phong Ngo 
 * This benchmark is part of TRACER
 */

/* Adapted from: https://github.com/sosy-lab/sv-benchmarks/blob/master/c/pthread-C-DAC/pthread-demo-datarace_true-unreach-call.c */





/***********************************************************************************
                         C-DAC Tech Workshop : hyPACK-2013
                             October 15-18,2013
  Example     : pthread-demo-datarace.c

  Objective   : Write Pthread code to illustrate Data Race Condition
            and its solution using MUTEX.

  Input       : Nothing.

  Output      : Value of Global variable with and without using Mutex.

  Created     :MAY-2013

  E-mail      : hpcfte@cdac.in

****************************************************************************/

/*
Modifications are made to remove non-standard libary dependencies by Yihao from
VSL of University of Delaware.
*/


#include "../verify.h"
#include <mutex>

#define THREADNUM 2
#define LOOP 10

// shared variables
atomic<int> myglobal; 
mutex mymutex;

void* thread_function_mutex1(void* arg)
{
  int i,j;
  // for ( i=0; i<LOOP; i++ )
  // {
  // unrolling: 1
    mymutex.lock();
    j = myglobal.load(memory_order_acquire);
    j=j+1;
    myglobal.store(j, memory_order_release);
    mymutex.unlock();
  // unrolling: 2
    mymutex.lock();
    j = myglobal.load(memory_order_acquire);
    j=j+1;
    myglobal.store(j, memory_order_release);
    mymutex.unlock();
  // unrolling: 3
    mymutex.lock();
    j = myglobal.load(memory_order_acquire);
    j=j+1;
    myglobal.store(j, memory_order_release);
    mymutex.unlock();
  // unrolling: 4
    mymutex.lock();
    j = myglobal.load(memory_order_acquire);
    j=j+1;
    myglobal.store(j, memory_order_release);
    mymutex.unlock();
  // unrolling: 5
    mymutex.lock();
    j = myglobal.load(memory_order_acquire);
    j=j+1;
    myglobal.store(j, memory_order_release);
    mymutex.unlock();
  // unrolling: 6
    mymutex.lock();
    j = myglobal.load(memory_order_acquire);
    j=j+1;
    myglobal.store(j, memory_order_release);
    mymutex.unlock();
  // unrolling: 7
    mymutex.lock();
    j = myglobal.load(memory_order_acquire);
    j=j+1;
    myglobal.store(j, memory_order_release);
    mymutex.unlock();
  // unrolling: 8
    mymutex.lock();
    j = myglobal.load(memory_order_acquire);
    j=j+1;
    myglobal.store(j, memory_order_release);
    mymutex.unlock();
  // unrolling: 9
    mymutex.lock();
    j = myglobal.load(memory_order_acquire);
    j=j+1;
    myglobal.store(j, memory_order_release);
    mymutex.unlock();
  // unrolling: 10
    mymutex.lock();
    j = myglobal.load(memory_order_acquire);
    j=j+1;
    myglobal.store(j, memory_order_release);
    mymutex.unlock();
  // }
	return NULL; 
}

void* thread_function_mutex2(void* arg)
{
  int i,j;
  // for ( i=0; i<LOOP; i++ )
  // {
  // unrolling: 1
    mymutex.lock();
    j = myglobal.load(memory_order_acquire);
    j=j+1;
    myglobal.store(j, memory_order_release);
    mymutex.unlock();
  // unrolling: 2
    mymutex.lock();
    j = myglobal.load(memory_order_acquire);
    j=j+1;
    myglobal.store(j, memory_order_release);
    mymutex.unlock();
  // unrolling: 3
    mymutex.lock();
    j = myglobal.load(memory_order_acquire);
    j=j+1;
    myglobal.store(j, memory_order_release);
    mymutex.unlock();
  // unrolling: 4
    mymutex.lock();
    j = myglobal.load(memory_order_acquire);
    j=j+1;
    myglobal.store(j, memory_order_release);
    mymutex.unlock();
  // unrolling: 5
    mymutex.lock();
    j = myglobal.load(memory_order_acquire);
    j=j+1;
    myglobal.store(j, memory_order_release);
    mymutex.unlock();
  // unrolling: 6
    mymutex.lock();
    j = myglobal.load(memory_order_acquire);
    j=j+1;
    myglobal.store(j, memory_order_release);
    mymutex.unlock();
  // unrolling: 7
    mymutex.lock();
    j = myglobal.load(memory_order_acquire);
    j=j+1;
    myglobal.store(j, memory_order_release);
    mymutex.unlock();
  // unrolling: 8
    mymutex.lock();
    j = myglobal.load(memory_order_acquire);
    j=j+1;
    myglobal.store(j, memory_order_release);
    mymutex.unlock();
  // unrolling: 9
    mymutex.lock();
    j = myglobal.load(memory_order_acquire);
    j=j+1;
    myglobal.store(j, memory_order_release);
    mymutex.unlock();
  // unrolling: 10
    mymutex.lock();
    j = myglobal.load(memory_order_acquire);
    j=j+1;
    myglobal.store(j, memory_order_release);
    mymutex.unlock();
  // }
	return NULL;
}
int main()
{
	pthread_t t1, t2;

	pthread_create(&t1, NULL, thread_function_mutex1, NULL);
	pthread_create(&t2, NULL, thread_function_mutex2, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
  
  return 0;
}

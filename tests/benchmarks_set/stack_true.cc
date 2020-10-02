/* Copyright (C) 2018 Tuan Phong Ngo 
 * This benchmark is part of TRACER
 */

/* Adapted from: https://github.com/sosy-lab/sv-benchmarks/blob/master/c/pthread/stack_true-unreach-call.c */

#include "../verify.h"
#include <mutex>

#define TRUE      (1)
#define FALSE     (0)
#define SIZE      (12)
#define OVERFLOW  (-1)
#define UNDERFLOW (-2)
#define FULL (-3)
#define NUM_PUSH_THREADS 1
#define NUM_POP_THREADS 1

// shared variables
atomic<int> top;
// atomic<int>  stack[SIZE];
atomic<int>  stack0, stack1, stack2, stack3,
			stack4, stack5, stack6, stack7,
			stack8, stack9, stack10, stack11;
mutex  mlock;

// void inc_top(void)
// {
//   int _top;
//   _top = top.load(memory_order_acquire);
//   top.store(_top+1, memory_order_release);
// }
// 
// void dec_top(void)
// {
//   int _top;
//   _top = top.load(memory_order_acquire);
//   top.store(_top-1, memory_order_release);
// }
// 
// int get_top(void)
// {
//   int _top;
//   _top = atomic_load_explicit(&top, memory_order_acquire);
//   return _top;
// }
// 
// int push(unsigned int x)
// {
//   int _top = top.load(memory_order_acquire);
//   if (_top > SIZE)
//   {
//     return OVERFLOW;
//   } else {
//     if (get_top() == SIZE) { // full state of the stack
//       return FULL;
//     }
// 	else {
//       _top = top.load(memory_order_acquire);
// 	  if (_top == 0) stack0.store(x, memory_order_release);
// 	  if (_top == 1) stack1.store(x, memory_order_release);
// 	  if (_top == 2) stack2.store(x, memory_order_release);
// 	  if (_top == 3) stack3.store(x, memory_order_release);
// 	  if (_top == 4) stack4.store(x, memory_order_release);
// 	  if (_top == 5) stack5.store(x, memory_order_release);
// 	  if (_top == 6) stack6.store(x, memory_order_release);
// 	  if (_top == 7) stack7.store(x, memory_order_release);
// 	  if (_top == 8) stack8.store(x, memory_order_release);
// 	  if (_top == 9) stack9.store(x, memory_order_release);
// 	  if (_top == 10) stack10.store(x, memory_order_release);
// 	  if (_top == 11) stack11.store(x, memory_order_release);
//       //inc_top();
//       _top = top.load(memory_order_acquire);
//       top.store(_top+1, memory_order_release);
// 	}
//   }
//   return 0;
// }
// 
// int pop(void)
// {
//   int _top = top.load(memory_order_acquire);
//   if (_top==0)
//   {
//     return UNDERFLOW;
//   }
//   else
//   {
//     // dec_top();
//     _top = top.load(memory_order_acquire);
//     top.store(_top-1, memory_order_release);
//     int _return;
// 	_top = top.load(memory_order_acquire);
// 	if (_top == 0) _return = stack0.load(memory_order_acquire);
// 	if (_top == 1) _return = stack1.load(memory_order_acquire);
// 	if (_top == 2) _return = stack2.load(memory_order_acquire);
// 	if (_top == 3) _return = stack3.load(memory_order_acquire);
// 	if (_top == 4) _return = stack4.load(memory_order_acquire);
// 	if (_top == 5) _return = stack5.load(memory_order_acquire);
// 	if (_top == 6) _return = stack6.load(memory_order_acquire);
// 	if (_top == 7) _return = stack7.load(memory_order_acquire);
// 	if (_top == 8) _return = stack8.load(memory_order_acquire);
// 	if (_top == 9) _return = stack9.load(memory_order_acquire);
// 	if (_top == 10) _return = stack10.load(memory_order_acquire);
// 	if (_top == 11) _return = stack11.load(memory_order_acquire);
//     return _return;
//   }
//   return 0;
// }


void* pushthread(void *arg)
{
  int i, tid;
  unsigned int tmp;
  tid = 0;

  // for(i=0; i<SIZE; i++)
  // {
    mlock.lock();
    int _top = top.load(memory_order_acquire);
    if (_top > SIZE)
    {
	  assert(0);
    } else {
	  _top = top.load(memory_order_acquire);
      if (_top == SIZE) { // full state of the stack
      }
      else {
        _top = top.load(memory_order_acquire);
        if (_top == 0) stack0.store(tid, memory_order_release);
        if (_top == 1) stack1.store(tid, memory_order_release);
        if (_top == 2) stack2.store(tid, memory_order_release);
        if (_top == 3) stack3.store(tid, memory_order_release);
        if (_top == 4) stack4.store(tid, memory_order_release);
        if (_top == 5) stack5.store(tid, memory_order_release);
        if (_top == 6) stack6.store(tid, memory_order_release);
        if (_top == 7) stack7.store(tid, memory_order_release);
        if (_top == 8) stack8.store(tid, memory_order_release);
        if (_top == 9) stack9.store(tid, memory_order_release);
        if (_top == 10) stack10.store(tid, memory_order_release);
        if (_top == 11) stack11.store(tid, memory_order_release);
        //inc_top();
        _top = top.load(memory_order_acquire);
        top.store(_top+1, memory_order_release);
      }
	}
    mlock.unlock();
  // }
  return NULL;
}

void* popthread(void *arg)
{
  int i, _top;

  // for(i=0; i<SIZE; i++)
  // {
    mlock.lock();
    _top = top.load(memory_order_acquire);
    if (_top > 0)
    {
      int _top = top.load(memory_order_acquire);
      if (_top==0)
      {
	    assert(0);
      }
      else
      {
        // dec_top();
        _top = top.load(memory_order_acquire);
        top.store(_top-1, memory_order_release);
        int _return;
        _top = top.load(memory_order_acquire);
        if (_top == 0) _return = stack0.load(memory_order_acquire);
        if (_top == 1) _return = stack1.load(memory_order_acquire);
        if (_top == 2) _return = stack2.load(memory_order_acquire);
        if (_top == 3) _return = stack3.load(memory_order_acquire);
        if (_top == 4) _return = stack4.load(memory_order_acquire);
        if (_top == 5) _return = stack5.load(memory_order_acquire);
        if (_top == 6) _return = stack6.load(memory_order_acquire);
        if (_top == 7) _return = stack7.load(memory_order_acquire);
        if (_top == 8) _return = stack8.load(memory_order_acquire);
        if (_top == 9) _return = stack9.load(memory_order_acquire);
        if (_top == 10) _return = stack10.load(memory_order_acquire);
        if (_top == 11) _return = stack11.load(memory_order_acquire);
      }
      // if (!(pop() != UNDERFLOW))
       // assert(0);
    }
    mlock.unlock();
  // }
  return NULL;
}

int main()
{
	// int i, arg[NUM_PUSH_THREADS];
	pthread_t t1s, t2s;

    pthread_create(&t1s, NULL, pushthread, NULL);
    pthread_create(&t2s, NULL, popthread, NULL);

	pthread_join(t1s, NULL);
	pthread_join(t2s, NULL);

  return 0;
}

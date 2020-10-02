/* Copyright (C) 2018 Tuan Phong Ngo 
 * This benchmark is part of TRACER
 */

/* Adapted from: https://github.com/sosy-lab/sv-benchmarks/blob/master/c/pthread-atomic/gcd_true-unreach-call_true-termination.c */

/*
VerifyThis ETAPS 2015, Challenge 2

PARALLEL GCD (60 minutes)
=========================

Algorithm description
---------------------

Various parallel GCD algorithms exist. In this challenge, we consider a
simple Euclid-like algorithm with two parallel threads. One thread
subtracts in one direction, the other thread subtracts in the other
direction, and eventually this procedure converges on GCD.


Implementation
--------------

In pseudocode, the algorithm is described as follows:

(
  WHILE a != b DO                                        
      IF a>b THEN a:=a-b ELSE SKIP FI
  OD
||
  WHILE a != b DO                                        
       IF b>a THEN b:=b-a ELSE SKIP FI
  OD
);
OUTPUT a


Verification tasks
------------------

Specify and verify the following behaviour of this parallel GCD algorithm:

Input:  two positive integers a and b
Output: a positive number that is the greatest common divisor of a and b

Feel free to add synchronisation where appropriate, but try to avoid
blocking of the parallel threads.


Sequentialization
-----------------

If your tool does not support reasoning about parallel threads, you may
verify the following pseudocode algorithm:

WHILE a != b DO
    CHOOSE(
         IF a > b THEN a := a - b ELSE SKIP FI,
         IF b > a THEN b := b - a ELSE SKIP FI
    )
OD;
OUTPUT a
*/

#include "../verify.h"
#include <mutex>

// #define A 17
// #define B 7
// #define LOOP 8

// shared variables
atomic_int a, b; 
mutex mlock;


// void atomic_dec_a(void)
// {
//   int _a, _b;
// 
//   mlock.lock();
//   _a = a.load(memory_order_acquire);
//   _b = b.load(memory_order_acquire);
//   if(_a > _b) {
//    _a = a.load(memory_order_acquire);
//    _b = b.load(memory_order_acquire);
//    a.store(_a -_b, memory_order_release);
//  }
//  mlock.unlock();
// }


// void atomic_dec_b(void)
// {
//   int _a, _b;
// 
//   mlock.lock();
//   _a = a.load(memory_order_acquire);
//   _b = b.load(memory_order_acquire);
//   if(_b > _a) {
//    _a = a.load(memory_order_acquire);
//    _b = b.load(memory_order_acquire);
//    b.store(_b - _a, memory_order_release);
//  }
//  mlock.unlock();
// }


void*  dec_a(void* arg)
{
  int _a, _b;
  // for (int i=0; i<LOOP; i++)
  // {
  // unrolling 1: 
   _a = a.load(memory_order_acquire);
   _b = b.load(memory_order_acquire);

  	if (_a != _b) // using for-if to have finite runs
  	{
      // inlining function call
	  // atomic_dec_a(); 
	  mlock.lock();
	  _a = a.load(memory_order_acquire);
	  _b = b.load(memory_order_acquire);
	  if(_a > _b) {
	    _a = a.load(memory_order_acquire);
	    _b = b.load(memory_order_acquire);
	    a.store(_a -_b, memory_order_release);
	  }
	  mlock.unlock();
	  // call ends here
      _a = a.load(memory_order_acquire);
      _b = b.load(memory_order_acquire);
      assert(_a >= 0); assert(_b>= 0);
    }
  // unrolling 2: 
   _a = a.load(memory_order_acquire);
   _b = b.load(memory_order_acquire);

  	if (_a != _b) // using for-if to have finite runs
  	{
      // inlining function call
	  // atomic_dec_a(); 
	  mlock.lock();
	  _a = a.load(memory_order_acquire);
	  _b = b.load(memory_order_acquire);
	  if(_a > _b) {
	    _a = a.load(memory_order_acquire);
	    _b = b.load(memory_order_acquire);
	    a.store(_a -_b, memory_order_release);
	  }
	  mlock.unlock();
	  // call ends here
      _a = a.load(memory_order_acquire);
      _b = b.load(memory_order_acquire);
      assert(_a >= 0); assert(_b>= 0);
    }
  // unrolling 3: 
   _a = a.load(memory_order_acquire);
   _b = b.load(memory_order_acquire);

  	if (_a != _b) // using for-if to have finite runs
  	{
      // inlining function call
	  // atomic_dec_a(); 
	  mlock.lock();
	  _a = a.load(memory_order_acquire);
	  _b = b.load(memory_order_acquire);
	  if(_a > _b) {
	    _a = a.load(memory_order_acquire);
	    _b = b.load(memory_order_acquire);
	    a.store(_a -_b, memory_order_release);
	  }
	  mlock.unlock();
	  // call ends here
      _a = a.load(memory_order_acquire);
      _b = b.load(memory_order_acquire);
      assert(_a >= 0); assert(_b>= 0);
    }
  // unrolling 4: 
   _a = a.load(memory_order_acquire);
   _b = b.load(memory_order_acquire);

  	if (_a != _b) // using for-if to have finite runs
  	{
      // inlining function call
	  // atomic_dec_a(); 
	  mlock.lock();
	  _a = a.load(memory_order_acquire);
	  _b = b.load(memory_order_acquire);
	  if(_a > _b) {
	    _a = a.load(memory_order_acquire);
	    _b = b.load(memory_order_acquire);
	    a.store(_a -_b, memory_order_release);
	  }
	  mlock.unlock();
	  // call ends here
      _a = a.load(memory_order_acquire);
      _b = b.load(memory_order_acquire);
      assert(_a >= 0); assert(_b>= 0);
    }
  // unrolling 5: 
   _a = a.load(memory_order_acquire);
   _b = b.load(memory_order_acquire);

  	if (_a != _b) // using for-if to have finite runs
  	{
      // inlining function call
	  // atomic_dec_a(); 
	  mlock.lock();
	  _a = a.load(memory_order_acquire);
	  _b = b.load(memory_order_acquire);
	  if(_a > _b) {
	    _a = a.load(memory_order_acquire);
	    _b = b.load(memory_order_acquire);
	    a.store(_a -_b, memory_order_release);
	  }
	  mlock.unlock();
	  // call ends here
      _a = a.load(memory_order_acquire);
      _b = b.load(memory_order_acquire);
      assert(_a >= 0); assert(_b>= 0);
    }
  // unrolling 6: 
   _a = a.load(memory_order_acquire);
   _b = b.load(memory_order_acquire);

  	if (_a != _b) // using for-if to have finite runs
  	{
      // inlining function call
	  // atomic_dec_a(); 
	  mlock.lock();
	  _a = a.load(memory_order_acquire);
	  _b = b.load(memory_order_acquire);
	  if(_a > _b) {
	    _a = a.load(memory_order_acquire);
	    _b = b.load(memory_order_acquire);
	    a.store(_a -_b, memory_order_release);
	  }
	  mlock.unlock();
	  // call ends here
      _a = a.load(memory_order_acquire);
      _b = b.load(memory_order_acquire);
      assert(_a >= 0); assert(_b>= 0);
    }
  // unrolling 7: 
   _a = a.load(memory_order_acquire);
   _b = b.load(memory_order_acquire);

  	if (_a != _b) // using for-if to have finite runs
  	{
      // inlining function call
	  // atomic_dec_a(); 
	  mlock.lock();
	  _a = a.load(memory_order_acquire);
	  _b = b.load(memory_order_acquire);
	  if(_a > _b) {
	    _a = a.load(memory_order_acquire);
	    _b = b.load(memory_order_acquire);
	    a.store(_a -_b, memory_order_release);
	  }
	  mlock.unlock();
	  // call ends here
      _a = a.load(memory_order_acquire);
      _b = b.load(memory_order_acquire);
      assert(_a >= 0); assert(_b>= 0);
    }
  // unrolling 8: 
   _a = a.load(memory_order_acquire);
   _b = b.load(memory_order_acquire);

  	if (_a != _b) // using for-if to have finite runs
  	{
      // inlining function call
	  // atomic_dec_a(); 
	  mlock.lock();
	  _a = a.load(memory_order_acquire);
	  _b = b.load(memory_order_acquire);
	  if(_a > _b) {
	    _a = a.load(memory_order_acquire);
	    _b = b.load(memory_order_acquire);
	    a.store(_a -_b, memory_order_release);
	  }
	  mlock.unlock();
	  // call ends here
      _a = a.load(memory_order_acquire);
      _b = b.load(memory_order_acquire);
      assert(_a >= 0); assert(_b>= 0);
    }
  // }
  return NULL;
}


void*  dec_b(void* arg)
{
  int _a, _b;

  // for (int i=0; i<LOOP; i++)
  // {
  // unrolling 1:
   _a = a.load(memory_order_acquire);
   _b = b.load(memory_order_acquire);

  	if (_a != _b) // using for-if to have finite runs
  	{
      // inlining function call
      // atomic_dec_b();
 	  mlock.lock();
 	  _a = a.load(memory_order_acquire);
 	  _b = b.load(memory_order_acquire);
 	  if(_b > _a) {
 	    _a = a.load(memory_order_acquire);
 	    _b = b.load(memory_order_acquire);
 	    b.store(_b - _a, memory_order_release);
 	  } 
 	  mlock.unlock();
      // call ends here
	  _a = a.load(memory_order_acquire);
      _b = b.load(memory_order_acquire);
      assert(_a >= 0); assert(_b >= 0);
    }
  // unrolling 2: 
   _a = a.load(memory_order_acquire);
   _b = b.load(memory_order_acquire);

  	if (_a != _b) // using for-if to have finite runs
  	{
      // inlining function call
      // atomic_dec_b();
 	  mlock.lock();
 	  _a = a.load(memory_order_acquire);
 	  _b = b.load(memory_order_acquire);
 	  if(_b > _a) {
 	    _a = a.load(memory_order_acquire);
 	    _b = b.load(memory_order_acquire);
 	    b.store(_b - _a, memory_order_release);
 	  } 
 	  mlock.unlock();
      // call ends here
	  _a = a.load(memory_order_acquire);
      _b = b.load(memory_order_acquire);
      assert(_a >= 0); assert(_b >= 0);
    }
  // unrolling 3: 
   _a = a.load(memory_order_acquire);
   _b = b.load(memory_order_acquire);

  	if (_a != _b) // using for-if to have finite runs
  	{
      // inlining function call
      // atomic_dec_b();
 	  mlock.lock();
 	  _a = a.load(memory_order_acquire);
 	  _b = b.load(memory_order_acquire);
 	  if(_b > _a) {
 	    _a = a.load(memory_order_acquire);
 	    _b = b.load(memory_order_acquire);
 	    b.store(_b - _a, memory_order_release);
 	  } 
 	  mlock.unlock();
      // call ends here
	  _a = a.load(memory_order_acquire);
      _b = b.load(memory_order_acquire);
      assert(_a >= 0); assert(_b >= 0);
    }
  // unrolling 4: 
   _a = a.load(memory_order_acquire);
   _b = b.load(memory_order_acquire);

  	if (_a != _b) // using for-if to have finite runs
  	{
      // inlining function call
      // atomic_dec_b();
 	  mlock.lock();
 	  _a = a.load(memory_order_acquire);
 	  _b = b.load(memory_order_acquire);
 	  if(_b > _a) {
 	    _a = a.load(memory_order_acquire);
 	    _b = b.load(memory_order_acquire);
 	    b.store(_b - _a, memory_order_release);
 	  } 
 	  mlock.unlock();
      // call ends here
	  _a = a.load(memory_order_acquire);
      _b = b.load(memory_order_acquire);
      assert(_a >= 0); assert(_b >= 0);
    }
  // unrolling 5: 
   _a = a.load(memory_order_acquire);
   _b = b.load(memory_order_acquire);

  	if (_a != _b) // using for-if to have finite runs
  	{
      // inlining function call
      // atomic_dec_b();
 	  mlock.lock();
 	  _a = a.load(memory_order_acquire);
 	  _b = b.load(memory_order_acquire);
 	  if(_b > _a) {
 	    _a = a.load(memory_order_acquire);
 	    _b = b.load(memory_order_acquire);
 	    b.store(_b - _a, memory_order_release);
 	  } 
 	  mlock.unlock();
      // call ends here
	  _a = a.load(memory_order_acquire);
      _b = b.load(memory_order_acquire);
      assert(_a >= 0); assert(_b >= 0);
    }
  // unrolling 6: 
   _a = a.load(memory_order_acquire);
   _b = b.load(memory_order_acquire);

  	if (_a != _b) // using for-if to have finite runs
  	{
      // inlining function call
      // atomic_dec_b();
 	  mlock.lock();
 	  _a = a.load(memory_order_acquire);
 	  _b = b.load(memory_order_acquire);
 	  if(_b > _a) {
 	    _a = a.load(memory_order_acquire);
 	    _b = b.load(memory_order_acquire);
 	    b.store(_b - _a, memory_order_release);
 	  } 
 	  mlock.unlock();
      // call ends here
	  _a = a.load(memory_order_acquire);
      _b = b.load(memory_order_acquire);
      assert(_a >= 0); assert(_b >= 0);
    }
  // unrolling 7: 
   _a = a.load(memory_order_acquire);
   _b = b.load(memory_order_acquire);

  	if (_a != _b) // using for-if to have finite runs
  	{
      // inlining function call
      // atomic_dec_b();
 	  mlock.lock();
 	  _a = a.load(memory_order_acquire);
 	  _b = b.load(memory_order_acquire);
 	  if(_b > _a) {
 	    _a = a.load(memory_order_acquire);
 	    _b = b.load(memory_order_acquire);
 	    b.store(_b - _a, memory_order_release);
 	  } 
 	  mlock.unlock();
      // call ends here
	  _a = a.load(memory_order_acquire);
      _b = b.load(memory_order_acquire);
      assert(_a >= 0); assert(_b >= 0);
    }
  // unrolling 8: 
   _a = a.load(memory_order_acquire);
   _b = b.load(memory_order_acquire);

  	if (_a != _b) // using for-if to have finite runs
  	{
      // inlining function call
      // atomic_dec_b();
 	  mlock.lock();
 	  _a = a.load(memory_order_acquire);
 	  _b = b.load(memory_order_acquire);
 	  if(_b > _a) {
 	    _a = a.load(memory_order_acquire);
 	    _b = b.load(memory_order_acquire);
 	    b.store(_b - _a, memory_order_release);
 	  } 
 	  mlock.unlock();
      // call ends here
	  _a = a.load(memory_order_acquire);
      _b = b.load(memory_order_acquire);
      assert(_a >= 0); assert(_b >= 0);
    }
  // }
  return NULL;
}


int main()
{
	a.store(17, memory_order_release);
	b.store(7, memory_order_release);

	pthread_t t1, t2;
	
	pthread_create(&t1, NULL, dec_a, NULL);
	pthread_create(&t2, NULL, dec_b, NULL);
 
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	
	return 0;
}

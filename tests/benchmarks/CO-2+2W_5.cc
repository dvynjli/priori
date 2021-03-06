/* Copyright (C) 2018 Tuan Phong Ngo
 * This file is part of TRACER */

/* There are N weak traces */

#include "../verify.h"

using namespace std;

#define N 5

atomic_int vars;

void *t1(void *arg){
  int tid = 1;
  vars.store(tid, memory_order_release);
  vars.store(tid, memory_order_release);
  return NULL;
}

void *t2(void *arg){
  int tid = 2;
  vars.store(tid, memory_order_release);
  vars.store(tid, memory_order_release);
  return NULL;
}

void *t3(void *arg){
  int tid = 3;
  vars.store(tid, memory_order_release);
  vars.store(tid, memory_order_release);
  return NULL;
}

void *t4(void *arg){
  int tid = 4;
  vars.store(tid, memory_order_release);
  vars.store(tid, memory_order_release);
  return NULL;
}

void *t5(void *arg){
  int tid = 5;
  vars.store(tid, memory_order_release);
  vars.store(tid, memory_order_release);
  return NULL;
}


int main(){
  vars.store(0, memory_order_release);
  pthread_t ts1, ts2, ts3, ts4, ts5;
  
  pthread_create(&ts1, NULL, t1, NULL);
  pthread_create(&ts2, NULL, t2, NULL);
  pthread_create(&ts3, NULL, t3, NULL);
  pthread_create(&ts4, NULL, t4, NULL);
  pthread_create(&ts5, NULL, t5, NULL);
  
  pthread_join(ts1, NULL);
  pthread_join(ts2, NULL);
  pthread_join(ts3, NULL);
  pthread_join(ts4, NULL);
  pthread_join(ts5, NULL);
  
  int v1 = vars.load(memory_order_acquire);
  int v2 = (v1 == N+1);
  if (v2 == 1) assert(0);
  return 0;
}

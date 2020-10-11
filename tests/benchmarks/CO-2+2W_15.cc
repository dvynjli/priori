/* Copyright (C) 2018 Tuan Phong Ngo
 * This file is part of TRACER */

/* There are N weak traces */

#include "../verify.h"

using namespace std;

#define N 15

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

void *t6(void *arg){
  int tid = 6;
  vars.store(tid, memory_order_release);
  vars.store(tid, memory_order_release);
  return NULL;
}

void *t7(void *arg){
  int tid = 7;
  vars.store(tid, memory_order_release);
  vars.store(tid, memory_order_release);
  return NULL;
}

void *t8(void *arg){
  int tid = 8;
  vars.store(tid, memory_order_release);
  vars.store(tid, memory_order_release);
  return NULL;
}

void *t9(void *arg){
  int tid = 9;
  vars.store(tid, memory_order_release);
  vars.store(tid, memory_order_release);
  return NULL;
}

void *t10(void *arg){
  int tid = 10;
  vars.store(tid, memory_order_release);
  vars.store(tid, memory_order_release);
  return NULL;
}


void *t11(void *arg){
  int tid = 11;
  vars.store(tid, memory_order_release);
  vars.store(tid, memory_order_release);
  return NULL;
}

void *t12(void *arg){
  int tid = 12;
  vars.store(tid, memory_order_release);
  vars.store(tid, memory_order_release);
  return NULL;
}

void *t13(void *arg){
  int tid = 13;
  vars.store(tid, memory_order_release);
  vars.store(tid, memory_order_release);
  return NULL;
}

void *t14(void *arg){
  int tid = 14;
  vars.store(tid, memory_order_release);
  vars.store(tid, memory_order_release);
  return NULL;
}

void *t15(void *arg){
  int tid = 15;
  vars.store(tid, memory_order_release);
  vars.store(tid, memory_order_release);
  return NULL;
}


int main(){
  vars.store(0, memory_order_release);
  pthread_t ts1, ts2, ts3, ts4, ts5,
        ts6, ts7, ts8, ts9, ts10,
        ts11, ts12, ts13, ts14, ts15;
  
  pthread_create(&ts1, NULL, t1, NULL);
  pthread_create(&ts2, NULL, t2, NULL);
  pthread_create(&ts3, NULL, t3, NULL);
  pthread_create(&ts4, NULL, t4, NULL);
  pthread_create(&ts5, NULL, t5, NULL);
  pthread_create(&ts6, NULL, t6, NULL);
  pthread_create(&ts7, NULL, t7, NULL);
  pthread_create(&ts8, NULL, t8, NULL);
  pthread_create(&ts9, NULL, t9, NULL);
  pthread_create(&ts10, NULL, t10, NULL);
  pthread_create(&ts11, NULL, t11, NULL);
  pthread_create(&ts12, NULL, t12, NULL);
  pthread_create(&ts13, NULL, t13, NULL);
  pthread_create(&ts14, NULL, t14, NULL);
  pthread_create(&ts15, NULL, t15, NULL);
  
  pthread_join(ts1, NULL);
  pthread_join(ts2, NULL);
  pthread_join(ts3, NULL);
  pthread_join(ts4, NULL);
  pthread_join(ts5, NULL);
  pthread_join(ts6, NULL);
  pthread_join(ts7, NULL);
  pthread_join(ts8, NULL);
  pthread_join(ts9, NULL);
  pthread_join(ts10, NULL);
  pthread_join(ts11, NULL);
  pthread_join(ts12, NULL);
  pthread_join(ts13, NULL);
  pthread_join(ts14, NULL);
  pthread_join(ts15, NULL);
  
  int v1 = vars.load(memory_order_acquire);
  int v2 = (v1 == N+1);
  if (v2 == 1) assert(0);
  return 0;
}

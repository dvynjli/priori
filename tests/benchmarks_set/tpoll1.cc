
#include "../verify.h"
#include <mutex>

#define NUM_THREADS 2
#define MAX_ITEMS 7

atomic<int> source0, source1, source2, source3, source4, source5, source6;
atomic<int> minBound0, minBound1;
atomic<int> maxBound0, maxBound1;
atomic<int> channel0, channel1;
atomic<int> th_id;

mutex mid;
mutex ms0, ms1;

void *sort_thread0 (void * arg)
{
   int id = -1;
   int x, y;

   // get an id
   // pthread_mutex_lock (&mid);
   // if locks are an issue, can replace it with faa
   mid.lock();
   id = th_id;
   th_id++;
   mid.unlock();
  
   return NULL;
}


void *sort_thread1 (void * arg)
{
   int id = -1;
   int x, y;

   // get an id
   // pthread_mutex_lock (&mid);
   // if locks are an issue, can replace it with faa
   mid.lock();
   id = th_id;
   th_id++;
   mid.unlock();
   
   return NULL;
}


int main ()
{
   pthread_t t0, t1;
   int i=0;

   th_id.store(0, REL);


   // this code initializes the source array with random numbers
   // i = __VERIFIER_nondet_int (0, MAX_ITEMS - 1);
   // source[i] = __VERIFIER_nondet_int (0, 20);
   // div: init all source 
   nondet_int(source0, 0, 20);
   nondet_int(source1, 0, 20);
   nondet_int(source2, 0, 20);
   nondet_int(source3, 0, 20);
   nondet_int(source4, 0, 20);
   nondet_int(source5, 0, 20);
   nondet_int(source6, 0, 20);


   int j = 0;
   int delta = 3; // MAX_ITEMS/NUM_THREADS;

   pthread_create (&t0, NULL, sort_thread0, NULL); i++;
   pthread_create(&t1, NULL, sort_thread1, NULL); i++;

assert(i == NUM_THREADS);

i = 0;
   pthread_join (t0, NULL); i++;
   pthread_join (t1, NULL); i++;
   return 0;
}


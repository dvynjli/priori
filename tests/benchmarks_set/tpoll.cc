
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

// void sort (int x, int y)
// {
//   int aux = 0;
//   for (int i = x; i < y; i++) 
//   {
//     for (int j = i; j < y; j++)
//     {
//       if (source[i] > source[j])
//       {
//         aux = source[i];
//         source[i] = source[j];
//         source[j] = aux;
//       }
//     }
//   }
// }

void *sort_thread0 (void * arg)
{
   int id = -1;
   int x, y;

   // get an id
   // pthread_mutex_lock (&mid);
   // if locks are an issue, can replace it with faa
   mid.lock();
   id = th_id.load(ACQ);
   th_id.fetch_add(1, ACQREL);
   mid.unlock();
   // pthread_mutex_unlock (&mid);

   // get my indexes
   if (id == 0) {
	   x = minBound0.load(ACQ);		// 0
	   y = maxBound0.load(ACQ);		// 2
   }
   else {
	   x = minBound1.load(ACQ);		// 3
	   y = maxBound1.load(ACQ);		// 6
   }
   
   // check bounds
   //@ assert (x >= 0);
   assert (x >= 0);
   //@ assert (x < MAX_ITEMS);
   assert (x < MAX_ITEMS);
   //@ assert (y >= 0);
   assert (y >= 0);
   //@ assert (y < MAX_ITEMS);
   assert (y < MAX_ITEMS);
   
   // sort
   // printf ("t%d: min %d max %d\n", id, x, y);
   // TODO: inline the function
   // sort (x, y);
   int aux = 0;
   int rs0, rs1, rs2;
   // t0: x=0, y=2
   // unfolding outer for loop
   // for (int i = x; i < y; i++) 
   int i = x;	// i=0
   if (i<y) {
     // unfolding inner for loop
     // for (int j = i; j < y; j++)
     int j=i;	// j=0
     if (j<y) {
       // i=0, j=0, if (source[i] > source[j])
       rs0 = source0.load(ACQ);
       if (rs0 > rs0) {
         // aux = source[i]; source[i] = source[j]; source[j] = aux;
         aux = source0.load(ACQ);
         source0.store(source0.load(ACQ), REL);
         source0.store(aux, REL);
       }
       j++; // j=1
       if (j<y) {
         // i=0, j=1, if (source[i] > source[j])
         rs0 = source0.load(ACQ);
         rs1 = source1.load(ACQ);
		     if (rs0 > rs0) {
		       // aux = source[i]; source[i] = source[j]; source[j] = aux;
		       aux = source0.load(ACQ);
		       source0.store(source1.load(ACQ), REL);
		       source1.store(aux, REL);
		     }
		     j++; // j=2
		     if (j<y) {
		     		// condition is false. skipping
		     }
       }
     }
     // inner for loop finished
     assume(!(j<y));
     i++;		// i=1
     // second itr of outer for loop
     if (i<y) {
		   // unfolding inner for loop
		   // for (int j = i; j < y; j++)
		   int j=i;	// j=1
	     if (j<y) {
	       // i=1, j=1, if (source[i] > source[j])
	       rs1 = source1.load(ACQ);
			   if (rs1 > rs1) {
			     // aux = source[i]; source[i] = source[j]; source[j] = aux;
			     aux = source1.load(ACQ);
			     source1.store(source1.load(ACQ), REL);
			     source1.store(aux, REL);
			   }
			   j++; // j=2
			   if (j<y) {
			   		// condition is false. skipping
			   }
	     }
		   // inner for loop finished
		   assume(!(j<y));
		   i++;		// i=2
		   // third itr of outer for loop
		   if (i<y) {	// condition is false. skipping
		 	 }
   	 }
   }
   assume (!(i<y));
   
   // I'm done
   // div: locks are not needed here, since the variable is atomic
   if (id == 0) {
	   ms0.lock();
	   channel0.store(1, REL);
	   ms0.unlock();
   }
   else {
	   ms1.lock();
	   channel1.store(1, REL);
	   ms1.unlock();
   }
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
   id = th_id.load(ACQ);
   th_id.fetch_add(1, ACQREL);
   mid.unlock();
   // pthread_mutex_unlock (&mid);

   // get my indexes
   if (id == 0) {
	   x = minBound0.load(ACQ);		// 0
	   y = maxBound0.load(ACQ);		// 2
   }
   else {
	   x = minBound1.load(ACQ);		// 3
	   y = maxBound1.load(ACQ);		// 6
   }
   
   // check bounds
   //@ assert (x >= 0);
   assert (x >= 0);
   //@ assert (x < MAX_ITEMS);
   assert (x < MAX_ITEMS);
   //@ assert (y >= 0);
   assert (y >= 0);
   //@ assert (y < MAX_ITEMS);
   assert (y < MAX_ITEMS);
   
   // sort
   // printf ("t%d: min %d max %d\n", id, x, y);
   // TODO: inline the function
   // sort (x, y);
   int aux = 0;
   int rs3, rs4, rs5;
   // t1: x=3, y=5
   // unfolding outer for loop
   // for (int i = x; i < y; i++) 
   int i = x;	// i=3
   if (i<y) {
     // unfolding inner for loop
     // for (int j = i; j < y; j++)
     int j=i;	// j=3
     if (j<y) {
       // i=3, j=3, if (source[i] > source[j])
       rs3 = source3.load(ACQ);
       if (rs3 > rs3) {
         // aux = source[i]; source[i] = source[j]; source[j] = aux;
         aux = source3.load(ACQ);
         source3.store(source0.load(ACQ), REL);
         source3.store(aux, REL);
       }
       j++; // j=4
       if (j<y) {
         // i=3, j=4, if (source[i] > source[j])
         rs3 = source3.load(ACQ);
         rs4 = source4.load(ACQ);
		     if (rs3 > rs4) {
		       // aux = source[i]; source[i] = source[j]; source[j] = aux;
		       aux = source3.load(ACQ);
		       source3.store(source4.load(ACQ), REL);
		       source4.store(aux, REL);
		     }
		     j++; // j=5
		     if (j<y) {
		     		// condition is false. skipping
		     }
       }
     }
     // inner for loop finished
     assume(!(j<y));
     i++;		// i=4
     // second itr of outer for loop
     if (i<y) {
		   // unfolding inner for loop
		   // for (int j = i; j < y; j++)
		   int j=i;	// j=4
	     if (j<y) {
	       // i=4, j=4, if (source[i] > source[j])
	       rs4 = source4.load(ACQ);
			   if (rs4 > rs4) {
			     // aux = source[i]; source[i] = source[j]; source[j] = aux;
			     aux = source4.load(ACQ);
			     source4.store(source4.load(ACQ), REL);
			     source4.store(aux, REL);
			   }
			   j++; // j=5
			   if (j<y) {
			   		// condition is false. skipping
			   }
	     }
		   // inner for loop finished
		   assume(!(j<y));
		   i++;		// i=5
		   // third itr of outer for loop
		   if (i<y) {	// condition is false. skipping
		 	 }
   	 }
   }
   assume (!(i<y));
   
   // I'm done
   // div: locks are not needed here, since the variable is atomic
   if (id == 0) {
	   ms0.lock();
	   channel0.store(1, REL);
	   ms0.unlock();
   }
   else {
	   ms1.lock();
	   channel1.store(1, REL);
	   ms1.unlock();
   }
   return NULL;
}


int main ()
{
   pthread_t t0, t1;
   int i;

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


   //@ assert (source[i] >= 0);
   // div: trivial assert. removing
   // __VERIFIER_assert (source[i] >= 0);

   // this code initializes the mutexes, the channel and the minBound and maxBounds
   int j = 0;
   int delta = 3; // MAX_ITEMS/NUM_THREADS;

   //@ assert (delta >= 1);
   // div: trivial assert. removing
   // __VERIFIER_assert (delta >= 1);

   // start threads (unfolded loop)
   i=0;
   channel0.store(0, REL);
   minBound0.store(j, REL);		// 0
   maxBound0.store(j+delta-1, REL);	// 0+3-1=2
   j+=delta;		// j=3
   pthread_create (&t0, NULL, sort_thread0, NULL);
   i++;
   channel1.store(0, REL);
   minBound1.store(j, REL); // 3
   maxBound1.store(j+delta-1, REL); // 3+3-1=5
   j+=delta;		// j=6
   pthread_create(&t1, NULL, sort_thread1, NULL);
   i++;

// #define ITER \
//    channel[i] = 0; \
//    minBound[i] = j; \
//    maxBound[i] = j + delta -1; \
//    j += delta; \
//    pthread_mutex_init (&ms[i], NULL); \
//    pthread_create (&t[i], NULL, sort_thread, NULL); \
//    i++;
//    ITER
//    ITER
// #undef ITER
   //@ assert (i == NUM_THREADS);
   assert(i == NUM_THREADS);

   // wait for all the threads to finish the sorting
//    int k = 0;
//    while (k < NUM_THREADS)
//    {
//      // check if any thread finished (unfolded loop)
//      // THIS LOOP CONTAINS A BUG
//      // The loop should terminate only after ALL thread have finished, but it
//      // can terminate after only ONE thread terminates, as it can increase k
//      // multiple times using the channel bit of that thread. As a result the
//      // assertions below can be violated
//      i = 0;
// #define ITER \
//      pthread_mutex_lock (&ms[i]); \
//      if (channel[i] == 1) { k++; } \
//      pthread_mutex_unlock (&ms[i]); \
//      i++;
//      ITER
//      ITER
// #undef ITER
//      //@ assert (i == NUM_THREADS);
//      __VERIFIER_assert (i == NUM_THREADS);
//    }

   // check that the correct number of threads was created
   //@ assert (th_id == NUM_THREADS);
   // assert (th_id == NUM_THREADS);

   // check that the correct number of threads has terminated 
   //@ assert (k == NUM_THREADS);
//    __VERIFIER_assert (k == NUM_THREADS);

   // merge the sorted arrays (we should merge here, instead of sorting again!!)
   // div: the sorted array is used only for printing here. Skipping.
   // sort (0, MAX_ITEMS);
   
 
   // print the sorted array
   // printf ("==============\n");
   // for (i = 0; i < MAX_ITEMS; i++)
   //    printf ("m: sorted[%d] = %d\n", i, source[i]);

   // join (all threads must have exited by now, due to the loop above, so if
   // the verifier does not support join, that won't impact the precission)
   i = 0;
   pthread_join (t0, NULL); i++;
   pthread_join (t1, NULL); i++;
   //@ assert (i == NUM_THREADS);
   assert (i == NUM_THREADS);

   return 0;
}


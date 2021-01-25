#include "../verify.h"
#include <mutex>

#define MAX_QUEUE 2
#define MAX_ITEMS 7

atomic_int q0, q1;
atomic_int qsiz;
mutex mq;

// void queue_init ()
// {
//    // pthread_mutex_init (&mq, NULL);
//    qsiz = 0;
// }
// 
// void queue_insert (int x)
// {
//    int done = 0;
//    // int i = 0;
//    // printf ("prod: trying\n");
//    // while (done == 0) // 
//    // {
//       // 
// 	  mq.lock();
//       int rqsiz = qsiz.load(ACQ);
// 	  if (rqsiz < MAX_QUEUE)
//       {
//          // printf ("prod: got it! x %d qsiz %d i %d\n", x, qsiz, i);
//          done = 1;
// 		 if (rqsiz == 0) {
// 			 q0.store(x, REL);
// 		 }
// 		 else {
// 			 q1.store(x, REL);
// 		 }
//          qsiz.fetch_add(1, ACQREL);
//       }
// 	  mq.unlock();
//    // }
//    // done can be 0 only if qsiz > MAX_QUEUE. TODO: check how is it possible
//    assume (done != 0);
// }
// 
// int queue_extract ()
// {
//    int done = 0;
//    int x = -1, i = 0;
//    // printf ("consumer: trying\n");
//    // while (done == 0)
//    // {
//    //   i++;
//    	  mq.lock();
// 	  int rqsiz = qsiz.load(ACQ);
//       if (rqsiz > 0)
//       {
//          done = 1;
//          x = q0.load(ACQ);
//    //      printf ("consumer: got it! x %d qsiz %d i %d\n", x, qsiz, i);
//          qsiz.fetch_sub(1, ACQREL);
// 		 rqsiz = qsiz.load(ACQ);
//          // for (i = 0; i < qsiz; i++) q[i] = q[i+1]; // shift left 1 elem
// 		 if (0 < qsiz) {
// 			 q0.store(q1, REL);
// 		 }
// 		 // TODO: check id qsiz can be > 1. Need to add more conditions if it is
// 		 rqsiz = qsiz.load(ACQ);
//          assert (rqsiz < MAX_QUEUE);
// 		 rqsiz = qsiz.load(ACQ);
// 		 if (qsiz == 0) {
// 			 q0.store(0, REL);
// 		 }
// 		 else {
// 			 q1.store(0, REL);
// 		 }
//       }
// 	  mq.unlock();
//    // }
//    assume(done!=0);
//    return x;
// }

// void swap (int *t, int i, int j)
// {
//    int aux;
//    aux = t[i];
//    t[i] = t[j];
//    t[j] = aux;
// }
// 
// int findmaxidx (int *t, int count)
// {
//    int i, mx;
//    mx = 0;
//    for (i = 1; i < count; i++)
//    {
//       if (t[i] > t[mx]) mx = i;
//    }
//    //@ assert (mx >= 0);
//    __VERIFIER_assert (mx >= 0);
// 
//    //@ assert (mx < count);
//    __VERIFIER_assert (mx < count);
// 
//    t[mx] = -t[mx];
//    return mx;
// }

atomic<int> source0, source1, source2, source3, source4, source5, source6;
atomic<int> sorted0, sorted1, sorted2, sorted3, sorted4, sorted5, sorted6;

void * producer (void * arg)
{
   int  idx;

   // for (i = 0; i < MAX_ITEMS; i++) 			//itr 1
   // {
      // idx = findmaxidx (source, MAX_ITEMS); 
	  // inlining with t = source, count = MAX_ITEMS
	  int mx=0, i = 1;
	  int rsmx;
	  if (i<MAX_ITEMS) {
		  int rs1 = source1.load(ACQ);
		  rsmx = source0.load(ACQ);
		  if (rs1 > rsmx) mx = i;
	  }
	  i++;	// i=2
	  if (i<MAX_ITEMS) {
		  int rs2 = source2.load(ACQ); 
		  if (mx == 0) // mx = [0,1]
			  rsmx = source0.load(ACQ);
		  else
			  rsmx = source1.load(ACQ);
		  if (rs2 > rsmx) mx = i;
	  }
	  i++; //i=3
	  if (i<MAX_ITEMS) {
		  int rs3 = source3.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else 
			  rsmx = source2.load(ACQ);
		  if (rs3 > rsmx) mx = i;
	  }
	  i++; //i=4
	  if (i<MAX_ITEMS) {
		  int rs4 = source4.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else if (mx == 2)
			  rsmx = source2.load(ACQ);
		  else 
			  rsmx = source3.load(ACQ);
		  if (rs4 > rsmx) mx = i;
	  }
	  i++; //i=5
	  if (i<MAX_ITEMS) {
		  int rs5 = source5.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else if (mx == 2)
			  rsmx = source2.load(ACQ);
		  else if (mx == 3)
			  rsmx = source3.load(ACQ);
		  else 
			  rsmx = source4.load(ACQ);
		  if (rs5 > rsmx) mx = i;
	  }
	  i++; //i=6
	  if (i<MAX_ITEMS) {
		  int rs6 = source6.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else if (mx == 2)
			  rsmx = source2.load(ACQ);
		  else if (mx == 3)
			  rsmx = source3.load(ACQ);
		  else if (mx == 4)
			  rsmx = source4.load(ACQ);
		  else 
			  rsmx = source5.load(ACQ);
		  if (rs6 > rsmx) mx = i;
	  }
	  i++; // i=7 > MAX_ITEMS loop terminated
	  assert(mx >= 0);
	  assert(mx < MAX_ITEMS);
	  
	  if (mx == 0) {
	      rsmx = source0.load(ACQ);
		  source0.store(-rsmx, REL);
	  } else if (mx == 1) {
	      rsmx = source1.load(ACQ);
		  source1.store(-rsmx, REL);
	  } else if (mx == 2) {
	      rsmx = source2.load(ACQ);
		  source2.store(-rsmx, REL); 
	  } else if (mx == 3) {
	      rsmx = source3.load(ACQ);
		  source3.store(-rsmx, REL);
	  } else if (mx == 4) {
	      rsmx = source4.load(ACQ);
		  source4.store(-rsmx, REL);
	  } else if (mx == 5) {
	      rsmx = source5.load(ACQ);
		  source5.store(-rsmx, REL);
	  } else {
		  rsmx = source6.load(ACQ);
		  source6.store(-rsmx, REL); 
	  }
	  idx = mx;  // return value
	  
      //@ assert (idx >= 0);
      assert (idx >= 0);

      //@ assert (idx < MAX_ITEMS);
      assert (idx < MAX_ITEMS);

      // queue_insert (idx); // inlining
   	  int done = 0;
   	  // int i = 0;
   	  // printf ("prod: trying\n");
   	  // while (done == 0) // 
   	  // {
   	     // 
   	     mq.lock();
   	     int rqsiz = qsiz.load(ACQ);
   	     if (rqsiz < MAX_QUEUE)
   	     {
   	        // printf ("prod: got it! x %d qsiz %d i %d\n", x, qsiz, i);
   	        done = 1;
   	   	 if (rqsiz == 0) {
   	   		 q0.store(idx, REL);
   	   	 }
   	   	 else {
   	   		 q1.store(idx, REL);
   	   	 }
   	        qsiz.fetch_add(1, ACQREL);
   	     }
   	     mq.unlock();
   	  // }
   	  // done can be 0 only if qsiz > MAX_QUEUE. TODO: check how is it possible
   	  assume (done != 0);
	  
   // }
   
   // for (i = 0; i < MAX_ITEMS; i++) 			//itr 2
   // {
      // idx = findmaxidx (source, MAX_ITEMS); 
	  // inlining with t = source, count = MAX_ITEMS
	  mx=0, i = 1;
	  if (i<MAX_ITEMS) {
		  int rs1 = source1.load(ACQ);
		  rsmx = source0.load(ACQ);
		  if (rs1 > rsmx) mx = i;
	  }
	  i++;	// i=2
	  if (i<MAX_ITEMS) {
		  int rs2 = source2.load(ACQ); 
		  if (mx == 0) // mx = [0,1]
			  rsmx = source0.load(ACQ);
		  else
			  rsmx = source1.load(ACQ);
		  if (rs2 > rsmx) mx = i;
	  }
	  i++; //i=3
	  if (i<MAX_ITEMS) {
		  int rs3 = source3.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else 
			  rsmx = source2.load(ACQ);
		  if (rs3 > rsmx) mx = i;
	  }
	  i++; //i=4
	  if (i<MAX_ITEMS) {
		  int rs4 = source4.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else if (mx == 2)
			  rsmx = source2.load(ACQ);
		  else 
			  rsmx = source3.load(ACQ);
		  if (rs4 > rsmx) mx = i;
	  }
	  i++; //i=5
	  if (i<MAX_ITEMS) {
		  int rs5 = source5.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else if (mx == 2)
			  rsmx = source2.load(ACQ);
		  else if (mx == 3)
			  rsmx = source3.load(ACQ);
		  else 
			  rsmx = source4.load(ACQ);
		  if (rs5 > rsmx) mx = i;
	  }
	  i++; //i=6
	  if (i<MAX_ITEMS) {
		  int rs6 = source6.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else if (mx == 2)
			  rsmx = source2.load(ACQ);
		  else if (mx == 3)
			  rsmx = source3.load(ACQ);
		  else if (mx == 4)
			  rsmx = source4.load(ACQ);
		  else 
			  rsmx = source5.load(ACQ);
		  if (rs6 > rsmx) mx = i;
	  }
	  i++; // i=7 > MAX_ITEMS loop terminated
	  assert(mx >= 0);
	  assert(mx < MAX_ITEMS);
	  
	  if (mx == 0) {
	      rsmx = source0.load(ACQ);
		  source0.store(-rsmx, REL);
	  } else if (mx == 1) {
	      rsmx = source1.load(ACQ);
		  source1.store(-rsmx, REL);
	  } else if (mx == 2) {
	      rsmx = source2.load(ACQ);
		  source2.store(-rsmx, REL); 
	  } else if (mx == 3) {
	      rsmx = source3.load(ACQ);
		  source3.store(-rsmx, REL);
	  } else if (mx == 4) {
	      rsmx = source4.load(ACQ);
		  source4.store(-rsmx, REL);
	  } else if (mx == 5) {
	      rsmx = source5.load(ACQ);
		  source5.store(-rsmx, REL);
	  } else {
		  rsmx = source6.load(ACQ);
		  source6.store(-rsmx, REL); 
	  }
	  idx = mx;  // return value
	  
      //@ assert (idx >= 0);
      assert (idx >= 0);

      //@ assert (idx < MAX_ITEMS);
      assert (idx < MAX_ITEMS);

      // queue_insert (idx); // inlining
   	  done = 0;
   	  // int i = 0;
   	  // printf ("prod: trying\n");
   	  // while (done == 0) // 
   	  // {
   	     // 
   	     mq.lock();
   	     rqsiz = qsiz.load(ACQ);
   	     if (rqsiz < MAX_QUEUE)
   	     {
   	        // printf ("prod: got it! x %d qsiz %d i %d\n", x, qsiz, i);
   	        done = 1;
   	   	 if (rqsiz == 0) {
   	   		 q0.store(idx, REL);
   	   	 }
   	   	 else {
   	   		 q1.store(idx, REL);
   	   	 }
   	        qsiz.fetch_add(1, ACQREL);
   	     }
   	     mq.unlock();
   	  // }
   	  // done can be 0 only if qsiz > MAX_QUEUE. TODO: check how is it possible
   	  assume (done != 0);
	  
   // }
   // for (i = 0; i < MAX_ITEMS; i++) 			//itr 3
   // {
      // idx = findmaxidx (source, MAX_ITEMS); 
	  // inlining with t = source, count = MAX_ITEMS
	  mx=0, i = 1;
	  if (i<MAX_ITEMS) {
		  int rs1 = source1.load(ACQ);
		  rsmx = source0.load(ACQ);
		  if (rs1 > rsmx) mx = i;
	  }
	  i++;	// i=2
	  if (i<MAX_ITEMS) {
		  int rs2 = source2.load(ACQ); 
		  if (mx == 0) // mx = [0,1]
			  rsmx = source0.load(ACQ);
		  else
			  rsmx = source1.load(ACQ);
		  if (rs2 > rsmx) mx = i;
	  }
	  i++; //i=3
	  if (i<MAX_ITEMS) {
		  int rs3 = source3.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else 
			  rsmx = source2.load(ACQ);
		  if (rs3 > rsmx) mx = i;
	  }
	  i++; //i=4
	  if (i<MAX_ITEMS) {
		  int rs4 = source4.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else if (mx == 2)
			  rsmx = source2.load(ACQ);
		  else 
			  rsmx = source3.load(ACQ);
		  if (rs4 > rsmx) mx = i;
	  }
	  i++; //i=5
	  if (i<MAX_ITEMS) {
		  int rs5 = source5.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else if (mx == 2)
			  rsmx = source2.load(ACQ);
		  else if (mx == 3)
			  rsmx = source3.load(ACQ);
		  else 
			  rsmx = source4.load(ACQ);
		  if (rs5 > rsmx) mx = i;
	  }
	  i++; //i=6
	  if (i<MAX_ITEMS) {
		  int rs6 = source6.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else if (mx == 2)
			  rsmx = source2.load(ACQ);
		  else if (mx == 3)
			  rsmx = source3.load(ACQ);
		  else if (mx == 4)
			  rsmx = source4.load(ACQ);
		  else 
			  rsmx = source5.load(ACQ);
		  if (rs6 > rsmx) mx = i;
	  }
	  i++; // i=7 > MAX_ITEMS loop terminated
	  assert(mx >= 0);
	  assert(mx < MAX_ITEMS);
	  
	  if (mx == 0) {
	      rsmx = source0.load(ACQ);
		  source0.store(-rsmx, REL);
	  } else if (mx == 1) {
	      rsmx = source1.load(ACQ);
		  source1.store(-rsmx, REL);
	  } else if (mx == 2) {
	      rsmx = source2.load(ACQ);
		  source2.store(-rsmx, REL); 
	  } else if (mx == 3) {
	      rsmx = source3.load(ACQ);
		  source3.store(-rsmx, REL);
	  } else if (mx == 4) {
	      rsmx = source4.load(ACQ);
		  source4.store(-rsmx, REL);
	  } else if (mx == 5) {
	      rsmx = source5.load(ACQ);
		  source5.store(-rsmx, REL);
	  } else {
		  rsmx = source6.load(ACQ);
		  source6.store(-rsmx, REL); 
	  }
	  idx = mx;  // return value
	  
      //@ assert (idx >= 0);
      assert (idx >= 0);

      //@ assert (idx < MAX_ITEMS);
      assert (idx < MAX_ITEMS);

      // queue_insert (idx); // inlining
   	  done = 0;
   	  // int i = 0;
   	  // printf ("prod: trying\n");
   	  // while (done == 0) // 
   	  // {
   	     // 
   	     mq.lock();
   	     rqsiz = qsiz.load(ACQ);
   	     if (rqsiz < MAX_QUEUE)
   	     {
   	        // printf ("prod: got it! x %d qsiz %d i %d\n", x, qsiz, i);
   	        done = 1;
   	   	 if (rqsiz == 0) {
   	   		 q0.store(idx, REL);
   	   	 }
   	   	 else {
   	   		 q1.store(idx, REL);
   	   	 }
   	        qsiz.fetch_add(1, ACQREL);
   	     }
   	     mq.unlock();
   	  // }
   	  // done can be 0 only if qsiz > MAX_QUEUE. TODO: check how is it possible
   	  assume (done != 0);
	  
   // }
   // for (i = 0; i < MAX_ITEMS; i++) 			//itr 4
   // {
      // idx = findmaxidx (source, MAX_ITEMS); 
	  // inlining with t = source, count = MAX_ITEMS
	  mx=0, i = 1;
	  if (i<MAX_ITEMS) {
		  int rs1 = source1.load(ACQ);
		  rsmx = source0.load(ACQ);
		  if (rs1 > rsmx) mx = i;
	  }
	  i++;	// i=2
	  if (i<MAX_ITEMS) {
		  int rs2 = source2.load(ACQ); 
		  if (mx == 0) // mx = [0,1]
			  rsmx = source0.load(ACQ);
		  else
			  rsmx = source1.load(ACQ);
		  if (rs2 > rsmx) mx = i;
	  }
	  i++; //i=3
	  if (i<MAX_ITEMS) {
		  int rs3 = source3.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else 
			  rsmx = source2.load(ACQ);
		  if (rs3 > rsmx) mx = i;
	  }
	  i++; //i=4
	  if (i<MAX_ITEMS) {
		  int rs4 = source4.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else if (mx == 2)
			  rsmx = source2.load(ACQ);
		  else 
			  rsmx = source3.load(ACQ);
		  if (rs4 > rsmx) mx = i;
	  }
	  i++; //i=5
	  if (i<MAX_ITEMS) {
		  int rs5 = source5.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else if (mx == 2)
			  rsmx = source2.load(ACQ);
		  else if (mx == 3)
			  rsmx = source3.load(ACQ);
		  else 
			  rsmx = source4.load(ACQ);
		  if (rs5 > rsmx) mx = i;
	  }
	  i++; //i=6
	  if (i<MAX_ITEMS) {
		  int rs6 = source6.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else if (mx == 2)
			  rsmx = source2.load(ACQ);
		  else if (mx == 3)
			  rsmx = source3.load(ACQ);
		  else if (mx == 4)
			  rsmx = source4.load(ACQ);
		  else 
			  rsmx = source5.load(ACQ);
		  if (rs6 > rsmx) mx = i;
	  }
	  i++; // i=7 > MAX_ITEMS loop terminated
	  assert(mx >= 0);
	  assert(mx < MAX_ITEMS);
	  
	  if (mx == 0) {
	      rsmx = source0.load(ACQ);
		  source0.store(-rsmx, REL);
	  } else if (mx == 1) {
	      rsmx = source1.load(ACQ);
		  source1.store(-rsmx, REL);
	  } else if (mx == 2) {
	      rsmx = source2.load(ACQ);
		  source2.store(-rsmx, REL); 
	  } else if (mx == 3) {
	      rsmx = source3.load(ACQ);
		  source3.store(-rsmx, REL);
	  } else if (mx == 4) {
	      rsmx = source4.load(ACQ);
		  source4.store(-rsmx, REL);
	  } else if (mx == 5) {
	      rsmx = source5.load(ACQ);
		  source5.store(-rsmx, REL);
	  } else {
		  rsmx = source6.load(ACQ);
		  source6.store(-rsmx, REL); 
	  }
	  idx = mx;  // return value
	  
      //@ assert (idx >= 0);
      assert (idx >= 0);

      //@ assert (idx < MAX_ITEMS);
      assert (idx < MAX_ITEMS);

      // queue_insert (idx); // inlining
   	  done = 0;
   	  // int i = 0;
   	  // printf ("prod: trying\n");
   	  // while (done == 0) // 
   	  // {
   	     // 
   	     mq.lock();
   	     rqsiz = qsiz.load(ACQ);
   	     if (rqsiz < MAX_QUEUE)
   	     {
   	        // printf ("prod: got it! x %d qsiz %d i %d\n", x, qsiz, i);
   	        done = 1;
   	   	 if (rqsiz == 0) {
   	   		 q0.store(idx, REL);
   	   	 }
   	   	 else {
   	   		 q1.store(idx, REL);
   	   	 }
   	        qsiz.fetch_add(1, ACQREL);
   	     }
   	     mq.unlock();
   	  // }
   	  // done can be 0 only if qsiz > MAX_QUEUE. TODO: check how is it possible
   	  assume (done != 0);
	  
   // }
   // for (i = 0; i < MAX_ITEMS; i++) 			//itr 5
   // {
      // idx = findmaxidx (source, MAX_ITEMS); 
	  // inlining with t = source, count = MAX_ITEMS
	  mx=0, i = 1;
	  if (i<MAX_ITEMS) {
		  int rs1 = source1.load(ACQ);
		  rsmx = source0.load(ACQ);
		  if (rs1 > rsmx) mx = i;
	  }
	  i++;	// i=2
	  if (i<MAX_ITEMS) {
		  int rs2 = source2.load(ACQ); 
		  if (mx == 0) // mx = [0,1]
			  rsmx = source0.load(ACQ);
		  else
			  rsmx = source1.load(ACQ);
		  if (rs2 > rsmx) mx = i;
	  }
	  i++; //i=3
	  if (i<MAX_ITEMS) {
		  int rs3 = source3.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else 
			  rsmx = source2.load(ACQ);
		  if (rs3 > rsmx) mx = i;
	  }
	  i++; //i=4
	  if (i<MAX_ITEMS) {
		  int rs4 = source4.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else if (mx == 2)
			  rsmx = source2.load(ACQ);
		  else 
			  rsmx = source3.load(ACQ);
		  if (rs4 > rsmx) mx = i;
	  }
	  i++; //i=5
	  if (i<MAX_ITEMS) {
		  int rs5 = source5.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else if (mx == 2)
			  rsmx = source2.load(ACQ);
		  else if (mx == 3)
			  rsmx = source3.load(ACQ);
		  else 
			  rsmx = source4.load(ACQ);
		  if (rs5 > rsmx) mx = i;
	  }
	  i++; //i=6
	  if (i<MAX_ITEMS) {
		  int rs6 = source6.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else if (mx == 2)
			  rsmx = source2.load(ACQ);
		  else if (mx == 3)
			  rsmx = source3.load(ACQ);
		  else if (mx == 4)
			  rsmx = source4.load(ACQ);
		  else 
			  rsmx = source5.load(ACQ);
		  if (rs6 > rsmx) mx = i;
	  }
	  i++; // i=7 > MAX_ITEMS loop terminated
	  assert(mx >= 0);
	  assert(mx < MAX_ITEMS);
	  
	  if (mx == 0) {
	      rsmx = source0.load(ACQ);
		  source0.store(-rsmx, REL);
	  } else if (mx == 1) {
	      rsmx = source1.load(ACQ);
		  source1.store(-rsmx, REL);
	  } else if (mx == 2) {
	      rsmx = source2.load(ACQ);
		  source2.store(-rsmx, REL); 
	  } else if (mx == 3) {
	      rsmx = source3.load(ACQ);
		  source3.store(-rsmx, REL);
	  } else if (mx == 4) {
	      rsmx = source4.load(ACQ);
		  source4.store(-rsmx, REL);
	  } else if (mx == 5) {
	      rsmx = source5.load(ACQ);
		  source5.store(-rsmx, REL);
	  } else {
		  rsmx = source6.load(ACQ);
		  source6.store(-rsmx, REL); 
	  }
	  idx = mx;  // return value
	  
      //@ assert (idx >= 0);
      assert (idx >= 0);

      //@ assert (idx < MAX_ITEMS);
      assert (idx < MAX_ITEMS);

      // queue_insert (idx); // inlining
   	  done = 0;
   	  // int i = 0;
   	  // printf ("prod: trying\n");
   	  // while (done == 0) // 
   	  // {
   	     // 
   	     mq.lock();
   	     rqsiz = qsiz.load(ACQ);
   	     if (rqsiz < MAX_QUEUE)
   	     {
   	        // printf ("prod: got it! x %d qsiz %d i %d\n", x, qsiz, i);
   	        done = 1;
   	   	 if (rqsiz == 0) {
   	   		 q0.store(idx, REL);
   	   	 }
   	   	 else {
   	   		 q1.store(idx, REL);
   	   	 }
   	        qsiz.fetch_add(1, ACQREL);
   	     }
   	     mq.unlock();
   	  // }
   	  // done can be 0 only if qsiz > MAX_QUEUE. TODO: check how is it possible
   	  assume (done != 0);
	  
   // }
   // for (i = 0; i < MAX_ITEMS; i++) 			//itr 6
   // {
      // idx = findmaxidx (source, MAX_ITEMS); 
	  // inlining with t = source, count = MAX_ITEMS
	  mx=0, i = 1;
	  if (i<MAX_ITEMS) {
		  int rs1 = source1.load(ACQ);
		  rsmx = source0.load(ACQ);
		  if (rs1 > rsmx) mx = i;
	  }
	  i++;	// i=2
	  if (i<MAX_ITEMS) {
		  int rs2 = source2.load(ACQ); 
		  if (mx == 0) // mx = [0,1]
			  rsmx = source0.load(ACQ);
		  else
			  rsmx = source1.load(ACQ);
		  if (rs2 > rsmx) mx = i;
	  }
	  i++; //i=3
	  if (i<MAX_ITEMS) {
		  int rs3 = source3.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else 
			  rsmx = source2.load(ACQ);
		  if (rs3 > rsmx) mx = i;
	  }
	  i++; //i=4
	  if (i<MAX_ITEMS) {
		  int rs4 = source4.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else if (mx == 2)
			  rsmx = source2.load(ACQ);
		  else 
			  rsmx = source3.load(ACQ);
		  if (rs4 > rsmx) mx = i;
	  }
	  i++; //i=5
	  if (i<MAX_ITEMS) {
		  int rs5 = source5.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else if (mx == 2)
			  rsmx = source2.load(ACQ);
		  else if (mx == 3)
			  rsmx = source3.load(ACQ);
		  else 
			  rsmx = source4.load(ACQ);
		  if (rs5 > rsmx) mx = i;
	  }
	  i++; //i=6
	  if (i<MAX_ITEMS) {
		  int rs6 = source6.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else if (mx == 2)
			  rsmx = source2.load(ACQ);
		  else if (mx == 3)
			  rsmx = source3.load(ACQ);
		  else if (mx == 4)
			  rsmx = source4.load(ACQ);
		  else 
			  rsmx = source5.load(ACQ);
		  if (rs6 > rsmx) mx = i;
	  }
	  i++; // i=7 > MAX_ITEMS loop terminated
	  assert(mx >= 0);
	  assert(mx < MAX_ITEMS);
	  
	  if (mx == 0) {
	      rsmx = source0.load(ACQ);
		  source0.store(-rsmx, REL);
	  } else if (mx == 1) {
	      rsmx = source1.load(ACQ);
		  source1.store(-rsmx, REL);
	  } else if (mx == 2) {
	      rsmx = source2.load(ACQ);
		  source2.store(-rsmx, REL); 
	  } else if (mx == 3) {
	      rsmx = source3.load(ACQ);
		  source3.store(-rsmx, REL);
	  } else if (mx == 4) {
	      rsmx = source4.load(ACQ);
		  source4.store(-rsmx, REL);
	  } else if (mx == 5) {
	      rsmx = source5.load(ACQ);
		  source5.store(-rsmx, REL);
	  } else {
		  rsmx = source6.load(ACQ);
		  source6.store(-rsmx, REL); 
	  }
	  idx = mx;  // return value
	  
      //@ assert (idx >= 0);
      assert (idx >= 0);

      //@ assert (idx < MAX_ITEMS);
      assert (idx < MAX_ITEMS);

      // queue_insert (idx); // inlining
   	  done = 0;
   	  // int i = 0;
   	  // printf ("prod: trying\n");
   	  // while (done == 0) // 
   	  // {
   	     // 
   	     mq.lock();
   	     rqsiz = qsiz.load(ACQ);
   	     if (rqsiz < MAX_QUEUE)
   	     {
   	        // printf ("prod: got it! x %d qsiz %d i %d\n", x, qsiz, i);
   	        done = 1;
   	   	 if (rqsiz == 0) {
   	   		 q0.store(idx, REL);
   	   	 }
   	   	 else {
   	   		 q1.store(idx, REL);
   	   	 }
   	        qsiz.fetch_add(1, ACQREL);
   	     }
   	     mq.unlock();
   	  // }
   	  // done can be 0 only if qsiz > MAX_QUEUE. TODO: check how is it possible
   	  assume (done != 0);
	  
   // }
   // for (i = 0; i < MAX_ITEMS; i++) 			//itr 7
   // {
      // idx = findmaxidx (source, MAX_ITEMS); 
	  // inlining with t = source, count = MAX_ITEMS
	  mx=0, i = 1;
	  if (i<MAX_ITEMS) {
		  int rs1 = source1.load(ACQ);
		  rsmx = source0.load(ACQ);
		  if (rs1 > rsmx) mx = i;
	  }
	  i++;	// i=2
	  if (i<MAX_ITEMS) {
		  int rs2 = source2.load(ACQ); 
		  if (mx == 0) // mx = [0,1]
			  rsmx = source0.load(ACQ);
		  else
			  rsmx = source1.load(ACQ);
		  if (rs2 > rsmx) mx = i;
	  }
	  i++; //i=3
	  if (i<MAX_ITEMS) {
		  int rs3 = source3.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else 
			  rsmx = source2.load(ACQ);
		  if (rs3 > rsmx) mx = i;
	  }
	  i++; //i=4
	  if (i<MAX_ITEMS) {
		  int rs4 = source4.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else if (mx == 2)
			  rsmx = source2.load(ACQ);
		  else 
			  rsmx = source3.load(ACQ);
		  if (rs4 > rsmx) mx = i;
	  }
	  i++; //i=5
	  if (i<MAX_ITEMS) {
		  int rs5 = source5.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else if (mx == 2)
			  rsmx = source2.load(ACQ);
		  else if (mx == 3)
			  rsmx = source3.load(ACQ);
		  else 
			  rsmx = source4.load(ACQ);
		  if (rs5 > rsmx) mx = i;
	  }
	  i++; //i=6
	  if (i<MAX_ITEMS) {
		  int rs6 = source6.load(ACQ);
		  if (mx == 0)
			  rsmx = source0.load(ACQ);
		  else if (mx == 1)
			  rsmx = source1.load(ACQ);
		  else if (mx == 2)
			  rsmx = source2.load(ACQ);
		  else if (mx == 3)
			  rsmx = source3.load(ACQ);
		  else if (mx == 4)
			  rsmx = source4.load(ACQ);
		  else 
			  rsmx = source5.load(ACQ);
		  if (rs6 > rsmx) mx = i;
	  }
	  i++; // i=7 > MAX_ITEMS loop terminated
	  assert(mx >= 0);
	  assert(mx < MAX_ITEMS);
	  
	  if (mx == 0) {
	      rsmx = source0.load(ACQ);
		  source0.store(-rsmx, REL);
	  } else if (mx == 1) {
	      rsmx = source1.load(ACQ);
		  source1.store(-rsmx, REL);
	  } else if (mx == 2) {
	      rsmx = source2.load(ACQ);
		  source2.store(-rsmx, REL); 
	  } else if (mx == 3) {
	      rsmx = source3.load(ACQ);
		  source3.store(-rsmx, REL);
	  } else if (mx == 4) {
	      rsmx = source4.load(ACQ);
		  source4.store(-rsmx, REL);
	  } else if (mx == 5) {
	      rsmx = source5.load(ACQ);
		  source5.store(-rsmx, REL);
	  } else {
		  rsmx = source6.load(ACQ);
		  source6.store(-rsmx, REL); 
	  }
	  idx = mx;  // return value
	  
      //@ assert (idx >= 0);
      assert (idx >= 0);

      //@ assert (idx < MAX_ITEMS);
      assert (idx < MAX_ITEMS);

      // queue_insert (idx); // inlining
   	  done = 0;
   	  // int i = 0;
   	  // printf ("prod: trying\n");
   	  // while (done == 0) // 
   	  // {
   	     // 
   	     mq.lock();
   	     rqsiz = qsiz.load(ACQ);
   	     if (rqsiz < MAX_QUEUE)
   	     {
   	        // printf ("prod: got it! x %d qsiz %d i %d\n", x, qsiz, i);
   	        done = 1;
   	   	 if (rqsiz == 0) {
   	   		 q0.store(idx, REL);
   	   	 }
   	   	 else {
   	   		 q1.store(idx, REL);
   	   	 }
   	        qsiz.fetch_add(1, ACQREL);
   	     }
   	     mq.unlock();
   	  // }
   	  // done can be 0 only if qsiz > MAX_QUEUE. TODO: check how is it possible
   	  assume (done != 0);
	  
   // }
   return NULL;
}

void * consumer (void * arg)
{
   int i=0, idx;
   // for (i = 0; i < MAX_ITEMS; i++) 			// itr 1
   // {
      // idx = queue_extract ();  // inlining
   int done = 0;
   int x = -1, j = 0;
   // printf ("consumer: trying\n");
   // while (done == 0)
   // {
   //   j++;
   	  mq.lock();
	  int rqsiz = qsiz.load(ACQ);
      if (rqsiz > 0)
      {
         done = 1;
         x = q0.load(ACQ);
   //      printf ("consumer: got it! x %d qsiz %d j %d\n", x, qsiz, j);
         qsiz.fetch_sub(1, ACQREL);
		 rqsiz = qsiz.load(ACQ);
         // for (j = 0; j < qsiz; j++) q[j] = q[j+1]; // shift left 1 elem
		 if (0 < rqsiz) {
			 int rq1 = q1.load(ACQ);
			 q0.store(rq1, REL);
		 }
		 // TODO: check id qsiz can be > 1. Need to add more conditions if it is
		 rqsiz = qsiz.load(ACQ);
         assert (rqsiz < MAX_QUEUE);
		 rqsiz = qsiz.load(ACQ);
		 if (rqsiz == 0) {
			 q0.store(0, REL);
		 }
		 else {
			 q1.store(0, REL);
		 }
      }
	  mq.unlock();
   // }
   assume(done!=0);
   // return x; 
   idx = x; // return value
      
      // sorted[i] = idx;      
      if (i==0) {
      	sorted0.store(idx, REL);
      } else if (i==1) {
      	sorted1.store(idx, REL);
      } else if (i==2) {
      	sorted2.store(idx, REL);
      } else if (i==3) {
      	sorted3.store(idx, REL);
      } else if (i==4) {
      	sorted4.store(idx, REL);
      } else if (i==5) {
      	sorted5.store(idx, REL);
      } else {		// 0 <=i < MAX_ITEMS 
      	sorted6.store(idx, REL);
      } 
      // printf ("m: i %d sorted = %d\n", i, sorted[i]);

      // global
      //@ assert (idx >= 0);
      assert (idx >= 0);

      // global
      //@ assert (idx < MAX_ITEMS);
      assert (idx < MAX_ITEMS);

      // global, requires relational domain, does not race; wont be able to
      // prove it with poet, frama-c or AstreA
      //__VERIFIER_assert (source[idx] < 0);
      ////@ assert (source[idx] < 0);
   // }
   
   // for (i = 0; i < MAX_ITEMS; i++) 			// itr 2
   // {
      // idx = queue_extract ();  // inlining
   done = 0;
   x = -1, j = 0;
   // printf ("consumer: trying\n");
   // while (done == 0)
   // {
   //   j++;
   	  mq.lock();
	  rqsiz = qsiz.load(ACQ);
      if (rqsiz > 0)
      {
         done = 1;
         x = q0.load(ACQ);
   //      printf ("consumer: got it! x %d qsiz %d j %d\n", x, qsiz, j);
         qsiz.fetch_sub(1, ACQREL);
		 rqsiz = qsiz.load(ACQ);
         // for (j = 0; j < qsiz; j++) q[j] = q[j+1]; // shift left 1 elem
		 if (0 < rqsiz) {
			 int rq1 = q1.load(ACQ);
			 q0.store(rq1, REL);
		 }
		 // TODO: check id qsiz can be > 1. Need to add more conditions if it is
		 rqsiz = qsiz.load(ACQ);
         assert (rqsiz < MAX_QUEUE);
		 rqsiz = qsiz.load(ACQ);
		 if (rqsiz == 0) {
			 q0.store(0, REL);
		 }
		 else {
			 q1.store(0, REL);
		 }
      }
	  mq.unlock();
   // }
   assume(done!=0);
   // return x; 
   idx = x; // return value
      
      // sorted[i] = idx;      
      if (i==0) {
      	sorted0.store(idx, REL);
      } else if (i==1) {
      	sorted1.store(idx, REL);
      } else if (i==2) {
      	sorted2.store(idx, REL);
      } else if (i==3) {
      	sorted3.store(idx, REL);
      } else if (i==4) {
      	sorted4.store(idx, REL);
      } else if (i==5) {
      	sorted5.store(idx, REL);
      } else {		// 0 <=i < MAX_ITEMS 
      	sorted6.store(idx, REL);
      } 
      // printf ("m: i %d sorted = %d\n", i, sorted[i]);

      // global
      //@ assert (idx >= 0);
      assert (idx >= 0);

      // global
      //@ assert (idx < MAX_ITEMS);
      assert (idx < MAX_ITEMS);

      // global, requires relational domain, does not race; wont be able to
      // prove it with poet, frama-c or AstreA
      //__VERIFIER_assert (source[idx] < 0);
      ////@ assert (source[idx] < 0);
   // }
   
   // for (i = 0; i < MAX_ITEMS; i++) 			// itr 3
   // {
      // idx = queue_extract ();  // inlining
   done = 0;
   x = -1, j = 0;
   // printf ("consumer: trying\n");
   // while (done == 0)
   // {
   //   j++;
   	  mq.lock();
	  rqsiz = qsiz.load(ACQ);
      if (rqsiz > 0)
      {
         done = 1;
         x = q0.load(ACQ);
   //      printf ("consumer: got it! x %d qsiz %d j %d\n", x, qsiz, j);
         qsiz.fetch_sub(1, ACQREL);
		 rqsiz = qsiz.load(ACQ);
         // for (j = 0; j < qsiz; j++) q[j] = q[j+1]; // shift left 1 elem
		 if (0 < rqsiz) {
			 int rq1 = q1.load(ACQ);
			 q0.store(rq1, REL);
		 }
		 // TODO: check id qsiz can be > 1. Need to add more conditions if it is
		 rqsiz = qsiz.load(ACQ);
         assert (rqsiz < MAX_QUEUE);
		 rqsiz = qsiz.load(ACQ);
		 if (rqsiz == 0) {
			 q0.store(0, REL);
		 }
		 else {
			 q1.store(0, REL);
		 }
      }
	  mq.unlock();
   // }
   assume(done!=0);
   // return x; 
   idx = x; // return value
      
      // sorted[i] = idx;      
      if (i==0) {
      	sorted0.store(idx, REL);
      } else if (i==1) {
      	sorted1.store(idx, REL);
      } else if (i==2) {
      	sorted2.store(idx, REL);
      } else if (i==3) {
      	sorted3.store(idx, REL);
      } else if (i==4) {
      	sorted4.store(idx, REL);
      } else if (i==5) {
      	sorted5.store(idx, REL);
      } else {		// 0 <=i < MAX_ITEMS 
      	sorted6.store(idx, REL);
      } 
      // printf ("m: i %d sorted = %d\n", i, sorted[i]);

      // global
      //@ assert (idx >= 0);
      assert (idx >= 0);

      // global
      //@ assert (idx < MAX_ITEMS);
      assert (idx < MAX_ITEMS);

      // global, requires relational domain, does not race; wont be able to
      // prove it with poet, frama-c or AstreA
      //__VERIFIER_assert (source[idx] < 0);
      ////@ assert (source[idx] < 0);
   // }
   
   // for (i = 0; i < MAX_ITEMS; i++) 			// itr 4
   // {
      // idx = queue_extract ();  // inlining
   done = 0;
   x = -1, j = 0;
   // printf ("consumer: trying\n");
   // while (done == 0)
   // {
   //   j++;
   	  mq.lock();
	  rqsiz = qsiz.load(ACQ);
      if (rqsiz > 0)
      {
         done = 1;
         x = q0.load(ACQ);
   //      printf ("consumer: got it! x %d qsiz %d j %d\n", x, qsiz, j);
         qsiz.fetch_sub(1, ACQREL);
		 rqsiz = qsiz.load(ACQ);
         // for (j = 0; j < qsiz; j++) q[j] = q[j+1]; // shift left 1 elem
		 if (0 < rqsiz) {
			 int rq1 = q1.load(ACQ);
			 q0.store(rq1, REL);
		 }
		 // TODO: check id qsiz can be > 1. Need to add more conditions if it is
		 rqsiz = qsiz.load(ACQ);
         assert (rqsiz < MAX_QUEUE);
		 rqsiz = qsiz.load(ACQ);
		 if (rqsiz == 0) {
			 q0.store(0, REL);
		 }
		 else {
			 q1.store(0, REL);
		 }
      }
	  mq.unlock();
   // }
   assume(done!=0);
   // return x; 
   idx = x; // return value
      
      // sorted[i] = idx;      
      if (i==0) {
      	sorted0.store(idx, REL);
      } else if (i==1) {
      	sorted1.store(idx, REL);
      } else if (i==2) {
      	sorted2.store(idx, REL);
      } else if (i==3) {
      	sorted3.store(idx, REL);
      } else if (i==4) {
      	sorted4.store(idx, REL);
      } else if (i==5) {
      	sorted5.store(idx, REL);
      } else {		// 0 <=i < MAX_ITEMS 
      	sorted6.store(idx, REL);
      } 
      // printf ("m: i %d sorted = %d\n", i, sorted[i]);

      // global
      //@ assert (idx >= 0);
      assert (idx >= 0);

      // global
      //@ assert (idx < MAX_ITEMS);
      assert (idx < MAX_ITEMS);

      // global, requires relational domain, does not race; wont be able to
      // prove it with poet, frama-c or AstreA
      //__VERIFIER_assert (source[idx] < 0);
      ////@ assert (source[idx] < 0);
   // }
   
   // for (i = 0; i < MAX_ITEMS; i++) 			// itr 5
   // {
      // idx = queue_extract ();  // inlining
   done = 0;
   x = -1, j = 0;
   // printf ("consumer: trying\n");
   // while (done == 0)
   // {
   //   j++;
   	  mq.lock();
	  rqsiz = qsiz.load(ACQ);
      if (rqsiz > 0)
      {
         done = 1;
         x = q0.load(ACQ);
   //      printf ("consumer: got it! x %d qsiz %d j %d\n", x, qsiz, j);
         qsiz.fetch_sub(1, ACQREL);
		 rqsiz = qsiz.load(ACQ);
         // for (j = 0; j < qsiz; j++) q[j] = q[j+1]; // shift left 1 elem
		 if (0 < rqsiz) {
			 int rq1 = q1.load(ACQ);
			 q0.store(rq1, REL);
		 }
		 // TODO: check id qsiz can be > 1. Need to add more conditions if it is
		 rqsiz = qsiz.load(ACQ);
         assert (rqsiz < MAX_QUEUE);
		 rqsiz = qsiz.load(ACQ);
		 if (rqsiz == 0) {
			 q0.store(0, REL);
		 }
		 else {
			 q1.store(0, REL);
		 }
      }
	  mq.unlock();
   // }
   assume(done!=0);
   // return x; 
   idx = x; // return value
      
      // sorted[i] = idx;      
      if (i==0) {
      	sorted0.store(idx, REL);
      } else if (i==1) {
      	sorted1.store(idx, REL);
      } else if (i==2) {
      	sorted2.store(idx, REL);
      } else if (i==3) {
      	sorted3.store(idx, REL);
      } else if (i==4) {
      	sorted4.store(idx, REL);
      } else if (i==5) {
      	sorted5.store(idx, REL);
      } else {		// 0 <=i < MAX_ITEMS 
      	sorted6.store(idx, REL);
      } 
      // printf ("m: i %d sorted = %d\n", i, sorted[i]);

      // global
      //@ assert (idx >= 0);
      assert (idx >= 0);

      // global
      //@ assert (idx < MAX_ITEMS);
      assert (idx < MAX_ITEMS);

      // global, requires relational domain, does not race; wont be able to
      // prove it with poet, frama-c or AstreA
      //__VERIFIER_assert (source[idx] < 0);
      ////@ assert (source[idx] < 0);
   // }
   
   // for (i = 0; i < MAX_ITEMS; i++) 			// itr 6
   // {
      // idx = queue_extract ();  // inlining
   done = 0;
   x = -1, j = 0;
   // printf ("consumer: trying\n");
   // while (done == 0)
   // {
   //   j++;
   	  mq.lock();
	  rqsiz = qsiz.load(ACQ);
      if (rqsiz > 0)
      {
         done = 1;
         x = q0.load(ACQ);
   //      printf ("consumer: got it! x %d qsiz %d j %d\n", x, qsiz, j);
         qsiz.fetch_sub(1, ACQREL);
		 rqsiz = qsiz.load(ACQ);
         // for (j = 0; j < qsiz; j++) q[j] = q[j+1]; // shift left 1 elem
		 if (0 < rqsiz) {
			 int rq1 = q1.load(ACQ);
			 q0.store(rq1, REL);
		 }
		 // TODO: check id qsiz can be > 1. Need to add more conditions if it is
		 rqsiz = qsiz.load(ACQ);
         assert (rqsiz < MAX_QUEUE);
		 rqsiz = qsiz.load(ACQ);
		 if (rqsiz == 0) {
			 q0.store(0, REL);
		 }
		 else {
			 q1.store(0, REL);
		 }
      }
	  mq.unlock();
   // }
   assume(done!=0);
   // return x; 
   idx = x; // return value
      
      // sorted[i] = idx;      
      if (i==0) {
      	sorted0.store(idx, REL);
      } else if (i==1) {
      	sorted1.store(idx, REL);
      } else if (i==2) {
      	sorted2.store(idx, REL);
      } else if (i==3) {
      	sorted3.store(idx, REL);
      } else if (i==4) {
      	sorted4.store(idx, REL);
      } else if (i==5) {
      	sorted5.store(idx, REL);
      } else {		// 0 <=i < MAX_ITEMS 
      	sorted6.store(idx, REL);
      } 
      // printf ("m: i %d sorted = %d\n", i, sorted[i]);

      // global
      //@ assert (idx >= 0);
      assert (idx >= 0);

      // global
      //@ assert (idx < MAX_ITEMS);
      assert (idx < MAX_ITEMS);

      // global, requires relational domain, does not race; wont be able to
      // prove it with poet, frama-c or AstreA
      //__VERIFIER_assert (source[idx] < 0);
      ////@ assert (source[idx] < 0);
   // }
   
   // for (i = 0; i < MAX_ITEMS; i++) 			// itr 7
   // {
      // idx = queue_extract ();  // inlining
   done = 0;
   x = -1, j = 0;
   // printf ("consumer: trying\n");
   // while (done == 0)
   // {
   //   j++;
   	  mq.lock();
	  rqsiz = qsiz.load(ACQ);
      if (rqsiz > 0)
      {
         done = 1;
         x = q0.load(ACQ);
   //      printf ("consumer: got it! x %d qsiz %d j %d\n", x, qsiz, j);
         qsiz.fetch_sub(1, ACQREL);
		 rqsiz = qsiz.load(ACQ);
         // for (j = 0; j < qsiz; j++) q[j] = q[j+1]; // shift left 1 elem
		 if (0 < rqsiz) {
			 int rq1 = q1.load(ACQ);
			 q0.store(rq1, REL);
		 }
		 // TODO: check id qsiz can be > 1. Need to add more conditions if it is
		 rqsiz = qsiz.load(ACQ);
         assert (rqsiz < MAX_QUEUE);
		 rqsiz = qsiz.load(ACQ);
		 if (rqsiz == 0) {
			 q0.store(0, REL);
		 }
		 else {
			 q1.store(0, REL);
		 }
      }
	  mq.unlock();
   // }
   assume(done!=0);
   // return x; 
   idx = x; // return value
      
      // sorted[i] = idx;      
      if (i==0) {
      	sorted0.store(idx, REL);
      } else if (i==1) {
      	sorted1.store(idx, REL);
      } else if (i==2) {
      	sorted2.store(idx, REL);
      } else if (i==3) {
      	sorted3.store(idx, REL);
      } else if (i==4) {
      	sorted4.store(idx, REL);
      } else if (i==5) {
      	sorted5.store(idx, REL);
      } else {		// 0 <=i < MAX_ITEMS 
      	sorted6.store(idx, REL);
      } 
      // printf ("m: i %d sorted = %d\n", i, sorted[i]);

      // global
      //@ assert (idx >= 0);
      assert (idx >= 0);

      // global
      //@ assert (idx < MAX_ITEMS);
      assert (idx < MAX_ITEMS);

      // global, requires relational domain, does not race; wont be able to
      // prove it with poet, frama-c or AstreA
      //__VERIFIER_assert (source[idx] < 0);
      ////@ assert (source[idx] < 0);
   // }
   return NULL;
}


int main ()
{
   pthread_t t_prod, t_cons;
   int i;

   // __libc_init_poet ();

   // initialize the source array
   // for (i = 0; i < MAX_ITEMS; i++)
   // {
   //    source[i] = __VERIFIER_nondet_int(0,20);
   //    printf ("m: init i %d source = %d\n", i, source[i]);
   //    //@ assert (source[i] >= 0);
   //    __VERIFIER_assert (source[i] >= 0);
   // }
   // I don't have non-det. So I will initialize it with some values in 0 to 20 range.
   int tmp;
   assume(0 <= tmp && tmp <= 20);
   nondet_int(source0, 0, 20);
   nondet_int(source1, 0, 20);
   nondet_int(source2, 0, 20);
   nondet_int(source3, 0, 20);
   nondet_int(source4, 0, 20);
   nondet_int(source5, 0, 20);
   nondet_int(source6, 0, 20);

   // initialize shared variables
   // queue_init (); 
   qsiz.store(0, REL);

   // create one thread and run the consummer in the main thread
   pthread_create (&t_prod, NULL, producer, NULL);
   pthread_create (&t_cons, NULL, consumer, NULL);
   // consumer ();

   // join
   pthread_join (t_prod, NULL);
   pthread_join (t_cons, NULL);
   return 0;
}


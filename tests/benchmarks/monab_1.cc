#include "../verify.h"
#include <mutex>

#define NUM_THREADS 5
#define MIN 0
#define MAX 1000

mutex m; 

atomic<int> s;

void* thr1(void* arg)
{
  int l;
  nondet_int(l, MIN, MAX);
  l = 4;

  m.lock();
  s.store(l, REL);
  int rs = s.load(ACQ);
  assert (rs == l);
  m.unlock();

  return NULL;
}

int main()
{
  // s = __VERIFIER_nondet_int(MIN, MAX);
  nondet_int(s, MIN, MAX);
  pthread_t t0, t1, t2, t3, t4;

  // Unfolded loop
  pthread_create (&t0, NULL, thr1, NULL);
  pthread_create (&t1, NULL, thr1, NULL);
  pthread_create (&t2, NULL, thr1, NULL);
  pthread_create (&t3, NULL, thr1, NULL);
  pthread_create (&t4, NULL, thr1, NULL); 

   pthread_join (t0, NULL);
   pthread_join (t1, NULL);
   pthread_join (t2, NULL);
   pthread_join (t3, NULL);
   pthread_join (t4, NULL); 

  return 0;
}


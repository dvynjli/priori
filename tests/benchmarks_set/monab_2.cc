#include "../verify.h"
#include <mutex>

#define NUM_THREADS 5
#define MIN 0
#define MAX 1000

mutex m; 

atomic<int> s;
atomic<int> l;

void* thr1(void* arg)
{
  m.lock();
  assert (l==0 || s==1);
  s = 1; // changed from s = s || 1
  l = 1; // overapproximates
  m.unlock();

  return NULL;
}

int main()
{
  // initialization
  s.store(0, REL);
  l.store(0, REL);
  // added this initialization
  s = __VERIFIER_nondet_int(MIN, MAX);
  pthread_t t[NUM_THREADS];
  pthread_mutex_init (&m, NULL);

   // Unfolded loop
   int i = 0;
   pthread_create (&t[i], NULL, thr1, NULL); i++;
   pthread_create (&t[i], NULL, thr1, NULL); i++;
   pthread_create (&t[i], NULL, thr1, NULL); i++;
   pthread_create (&t[i], NULL, thr1, NULL); i++;
   pthread_create (&t[i], NULL, thr1, NULL); 

#if 0
   int i = 0;
   pthread_join (t[i], NULL); i++;
   pthread_join (t[i], NULL); i++;
   pthread_join (t[i], NULL); i++;
   pthread_join (t[i], NULL); i++;
   pthread_join (t[i], NULL); 
#endif

  return 0;
}


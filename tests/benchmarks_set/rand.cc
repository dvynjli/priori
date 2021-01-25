//http://www.ibm.com/developerworks/java/library/j-jtp11234/
//Listing 5. Implementing a thread-safe PRNG with synchronization and atomic variables

#include "../verify.h"
#include <mutex>

#define NUM_THREADS 5
#define MIN 0
#define MAX 1000
#define NUM 10

mutex m; 

atomic<int> seed;

void* thr0(void* arg){
  // res = PseudoRandomUsingAtomic_nex();
  int nex, nexts, casret, nex_return, nC_return;
  casret = 0;

  // unroll and assume
  // while(casret == 0) {
    // m.lock();
    nex = seed.load(ACQ);
    // m.unlock();
   
    // nexts = nC(nex);
	nondet_int(nC_return, MIN, MAX);
	
	// unroll and assume
    // while (nC_return == nex || nC_return == 0)
    // {
	  nondet_int(nC_return, MIN, MAX);
    // }
	assume(!(nC_return == nex || nC_return == 0));
    nexts = nC_return; 
    
    m.lock();
	// casret = __VERIFIER_atomic_CAS(nex,nexts);
	int rseed = seed.load(ACQ);
    if (rseed == nex)
    {
      seed.store(nexts, REL);
	  casret = 1;
    }
    else
    {
      casret = 0;
    }
	m.unlock();
  // }
  assume(!(casret == 0));
 
  if (nexts < NUM)
  {
    nex_return = nexts;
  }
  else 
  {
    nex_return = NUM;
  }

  assert (nex_return <= NUM);

  return NULL;
}

void* thr1(void* arg){
  // res = PseudoRandomUsingAtomic_nex();
  int nex, nexts, casret, nex_return, nC_return;
  casret = 0;

  // unroll and assume
  // while(casret == 0) {
    // m.lock();
    nex = seed.load(ACQ);
    // m.unlock();
   
    // nexts = nC(nex);
	nondet_int(nC_return, MIN, MAX);
	
	// unroll and assume
    // while (nC_return == nex || nC_return == 0)
    // {
	  nondet_int(nC_return, MIN, MAX);
    // }
	assume(!(nC_return == nex || nC_return == 0));
    nexts = nC_return; 
    
    m.lock();
	// casret = __VERIFIER_atomic_CAS(nex,nexts);
	int rseed = seed.load(ACQ);
    if (rseed == nex)
    {
      seed.store(nexts, REL);
	  casret = 1;
    }
    else
    {
      casret = 0;
    }
	m.unlock();
  // }
  assume(!(casret == 0));
 
  if (nexts < NUM)
  {
    nex_return = nexts;
  }
  else 
  {
    nex_return = NUM;
  }

  assert (nex_return <= NUM);

  return NULL;
}

void* thr2(void* arg){
  // res = PseudoRandomUsingAtomic_nex();
  int nex, nexts, casret, nex_return, nC_return;
  casret = 0;

  // unroll and assume
  // while(casret == 0) {
    // m.lock();
    nex = seed.load(ACQ);
    // m.unlock();
   
    // nexts = nC(nex);
	nondet_int(nC_return, MIN, MAX);
	
	// unroll and assume
    // while (nC_return == nex || nC_return == 0)
    // {
	  nondet_int(nC_return, MIN, MAX);
    // }
	assume(!(nC_return == nex || nC_return == 0));
    nexts = nC_return; 
    
    m.lock();
	// casret = __VERIFIER_atomic_CAS(nex,nexts);
	int rseed = seed.load(ACQ);
    if (rseed == nex)
    {
      seed.store(nexts, REL);
	  casret = 1;
    }
    else
    {
      casret = 0;
    }
	m.unlock();
  // }
  assume(!(casret == 0));
 
  if (nexts < NUM)
  {
    nex_return = nexts;
  }
  else 
  {
    nex_return = NUM;
  }

  assert (nex_return <= NUM);

  return NULL;
}

void* thr3(void* arg){
  // res = PseudoRandomUsingAtomic_nex();
  int nex, nexts, casret, nex_return, nC_return;
  casret = 0;

  // unroll and assume
  // while(casret == 0) {
    m.lock();
    nex = seed.load(ACQ);
    m.unlock();
   
    // nexts = nC(nex);
	nondet_int(nC_return, MIN, MAX);
	
	// unroll and assume
    // while (nC_return == nex || nC_return == 0)
    // {
	  nondet_int(nC_return, MIN, MAX);
    // }
	assume(!(nC_return == nex || nC_return == 0));
    nexts = nC_return; 
    
    m.lock();
	// casret = __VERIFIER_atomic_CAS(nex,nexts);
	int rseed = seed.load(ACQ);
    if (rseed == nex)
    {
      seed.store(nexts, REL);
	  casret = 1;
    }
    else
    {
      casret = 0;
    }
	m.unlock();
  // }
  assume(!(casret == 0));
 
  if (nexts < NUM)
  {
    nex_return = nexts;
  }
  else 
  {
    nex_return = NUM;
  }

  assert (nex_return <= NUM);

  return NULL;
}

void* thr4(void* arg){
  // res = PseudoRandomUsingAtomic_nex();
  int nex, nexts, casret, nex_return, nC_return;
  casret = 0;

  // unroll and assume
  // while(casret == 0) {
    m.lock();
    nex = seed.load(ACQ);
    m.unlock();
   
    // nexts = nC(nex);
	nondet_int(nC_return, MIN, MAX);
	
	// unroll and assume
    // while (nC_return == nex || nC_return == 0)
    // {
	  nondet_int(nC_return, MIN, MAX);
    // }
	assume(!(nC_return == nex || nC_return == 0));
    nexts = nC_return; 
    
    m.lock();
	// casret = __VERIFIER_atomic_CAS(nex,nexts);
	int rseed = seed.load(ACQ);
    if (rseed == nex)
    {
      seed.store(nexts, REL);
	  casret = 1;
    }
    else
    {
      casret = 0;
    }
	m.unlock();
  // }
  assume(!(casret == 0));
 
  if (nexts < NUM)
  {
    nex_return = nexts;
  }
  else 
  {
    nex_return = NUM;
  }

  assert (nex_return <= NUM);

  return NULL;
}

int main()
{
  pthread_t t0, t1, t2, t3, t4;

  seed.store(1, REL);

  // Unfolded loop
  pthread_create (&t0, NULL, thr0, NULL);
  pthread_create (&t1, NULL, thr1, NULL);
  pthread_create (&t2, NULL, thr2, NULL);
  // pthread_create (&t3, NULL, thr3, NULL);
  // pthread_create (&t4, NULL, thr4, NULL); 

  pthread_join (t0, NULL);
  pthread_join (t1, NULL);
  pthread_join (t2, NULL);
  // pthread_join (t3, NULL);
  // pthread_join (t4, NULL); 

  return 0;
}



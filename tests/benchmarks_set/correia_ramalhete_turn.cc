/* Implemented under the RA semantics
 * This benchmark is part of TRACER
 * Need to use fences under RA to avoid 
 * a violation of mutual execlusion property
 */

// Correia and Ramalhete CRTurn, Mutual Exclusion - Two linear wait software solutions
// https://github.com/pramalhe/ConcurrencyFreaks/tree/master/papers/cralgorithm-2015.pdf
//
// Shared words      = N+1
// Number of states  = 3
// Starvation-Free   = yes, with N
// Minimum SC stores = 1 + 1
// Minimum SC loads  = N+2

#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <atomic>

using namespace std;

#define LOOP 2		
#define N 3

#define UNLOCKED 0
#define WAITING 1
#define LOCKED 2
//enum State { UNLOCKED, WAITING, LOCKED };

#define CACHE_LINE  64
#define PADRATIO    (CACHE_LINE/sizeof(atomic_int))

// shared variables
// atomic_int states[N*PADRATIO]; 
atomic<int> states0, states1, states2, states3, states4, states5;
atomic_int turn;
atomic_int var;

//atomic_int __fence_var;

inline static int validate_left(int id, int lturn) {
    int i;
    if (lturn > id) {
        for (i = lturn; i < N; i++) {
            if (atomic_load_explicit(&states[i*PADRATIO], memory_order_acquire) != UNLOCKED) return 0;
        }
        for (i = 0; i < id; i++) {
            if (atomic_load_explicit(&states[i*PADRATIO], memory_order_acquire) != UNLOCKED) return 0;
        }
    } else {
        for (i = lturn; i < id; i++) {
            if (atomic_load_explicit(&states[i*PADRATIO], memory_order_acquire) != UNLOCKED) return 0;
        }
    }
    return 1;
}

inline static int validate_right(int id, int lturn) {
    int i;
    if (lturn <= id) {
        for (i = id + 1; i < N; i++) {
            if (atomic_load_explicit(&states[i*PADRATIO], memory_order_acquire) == LOCKED) return 0;
        }
        for (i = 0; i < lturn; i++) {
            if (atomic_load_explicit(&states[i*PADRATIO], memory_order_acquire) == LOCKED) return 0;
        }
    } else {
        for (i = id + 1; i < lturn; i++) {
            if (atomic_load_explicit(&states[i*PADRATIO], memory_order_acquire) == LOCKED) return 0;
        }
    }
    return 1;
}

void* Worker0(void *arg)
{
  	int id = 0;
	int ok;

    states0.store(LOCKED,  memory_order_release);

   	//atomic_fetch_add_explicit(&__fence_var, 0, memory_order_acq_rel);

    for (int j=0; j<LOOP; j++) {
        ok = 0;
        int lturn = turn.load(memory_order_acquire);
        if (validate_left(id, lturn)==0) {
            atomic_store_explicit(&states[id*PADRATIO], WAITING, memory_order_release);
            for (int jj=0; jj<LOOP; jj++) {
                if (validate_left(id, lturn)==1 && lturn == atomic_load_explicit(&turn, memory_order_acquire)) {
   					ok = 1;
                	break;
                }
                lturn = atomic_load_explicit(&turn, memory_order_acquire);
            }
            if (ok==0) return NULL;
            atomic_store_explicit(&states[id*PADRATIO], LOCKED, memory_order_release);
            continue;
        }

       	//atomic_fetch_add_explicit(&__fence_var, 0, memory_order_acq_rel);

        ok = 0;
        for (int jj=0; jj<LOOP; jj++) {
        	if (lturn != atomic_load_explicit(&turn, memory_order_acquire) ||
        		validate_right(id, lturn)==1) {
        		ok = 1;
        		break; 
        	}
        }

        if (ok==0) return NULL;
		ok = 0;
        if (lturn == atomic_load_explicit(&turn, memory_order_acquire)) 
		{
			ok = 1;	
			break;
		}
    }

	if (ok==1) {
		// critical section
		atomic_store_explicit(&var, id, memory_order_release);
		assert(atomic_load_explicit(&var, memory_order_acquire) == id);

		int lturn = (atomic_load_explicit(&turn, memory_order_acquire)+1) % N;
		atomic_store_explicit(&turn, lturn, memory_order_release);
		atomic_store_explicit(&states[id*PADRATIO], UNLOCKED, memory_order_release); // exit protocol
	}
	
	return NULL;

} // Worker

void* Worker1(void *arg)
{
  	int id = *((int *)arg);
	int ok;

    atomic_store_explicit(&states[id*PADRATIO], LOCKED,  memory_order_release);

   	//atomic_fetch_add_explicit(&__fence_var, 0, memory_order_acq_rel);

    for (int j=0; j<LOOP; j++) {
        ok = 0;
        int lturn = atomic_load_explicit(&turn, memory_order_acquire);
        if (validate_left(id, lturn)==0) {
            atomic_store_explicit(&states[id*PADRATIO], WAITING, memory_order_release);
            for (int jj=0; jj<LOOP; jj++) {
                if (validate_left(id, lturn)==1 && lturn == atomic_load_explicit(&turn, memory_order_acquire)) {
   					ok = 1;
                	break;
                }
                lturn = atomic_load_explicit(&turn, memory_order_acquire);
            }
            if (ok==0) return NULL;
            atomic_store_explicit(&states[id*PADRATIO], LOCKED, memory_order_release);
            continue;
        }

       	//atomic_fetch_add_explicit(&__fence_var, 0, memory_order_acq_rel);

        ok = 0;
        for (int jj=0; jj<LOOP; jj++) {
        	if (lturn != atomic_load_explicit(&turn, memory_order_acquire) ||
        		validate_right(id, lturn)==1) {
        		ok = 1;
        		break; 
        	}
        }

        if (ok==0) return NULL;
		ok = 0;
        if (lturn == atomic_load_explicit(&turn, memory_order_acquire)) 
		{
			ok = 1;	
			break;
		}
    }

	if (ok==1) {
		// critical section
		atomic_store_explicit(&var, id, memory_order_release);
		assert(atomic_load_explicit(&var, memory_order_acquire) == id);

		int lturn = (atomic_load_explicit(&turn, memory_order_acquire)+1) % N;
		atomic_store_explicit(&turn, lturn, memory_order_release);
		atomic_store_explicit(&states[id*PADRATIO], UNLOCKED, memory_order_release); // exit protocol
	}
	
	return NULL;

} // Worker


void* Worker2(void *arg)
{
  	int id = *((int *)arg);
	int ok;

    atomic_store_explicit(&states[id*PADRATIO], LOCKED,  memory_order_release);

   	//atomic_fetch_add_explicit(&__fence_var, 0, memory_order_acq_rel);

    for (int j=0; j<LOOP; j++) {
        ok = 0;
        int lturn = atomic_load_explicit(&turn, memory_order_acquire);
        if (validate_left(id, lturn)==0) {
            atomic_store_explicit(&states[id*PADRATIO], WAITING, memory_order_release);
            for (int jj=0; jj<LOOP; jj++) {
                if (validate_left(id, lturn)==1 && lturn == atomic_load_explicit(&turn, memory_order_acquire)) {
   					ok = 1;
                	break;
                }
                lturn = atomic_load_explicit(&turn, memory_order_acquire);
            }
            if (ok==0) return NULL;
            atomic_store_explicit(&states[id*PADRATIO], LOCKED, memory_order_release);
            continue;
        }

       	//atomic_fetch_add_explicit(&__fence_var, 0, memory_order_acq_rel);

        ok = 0;
        for (int jj=0; jj<LOOP; jj++) {
        	if (lturn != atomic_load_explicit(&turn, memory_order_acquire) ||
        		validate_right(id, lturn)==1) {
        		ok = 1;
        		break; 
        	}
        }

        if (ok==0) return NULL;
		ok = 0;
        if (lturn == atomic_load_explicit(&turn, memory_order_acquire)) 
		{
			ok = 1;	
			break;
		}
    }

	if (ok==1) {
		// critical section
		atomic_store_explicit(&var, id, memory_order_release);
		assert(atomic_load_explicit(&var, memory_order_acquire) == id);

		int lturn = (atomic_load_explicit(&turn, memory_order_acquire)+1) % N;
		atomic_store_explicit(&turn, lturn, memory_order_release);
		atomic_store_explicit(&states[id*PADRATIO], UNLOCKED, memory_order_release); // exit protocol
	}
	
	return NULL;

} // Worker

int main()
{
  	pthread_t ts[N];
  	int arg[N];

	for (int i=0; i<N; i++) {
	  	arg[i] = i;
	  	pthread_create(&ts[i], NULL, Worker, &arg[i]);
	}
  
  	for (int i=0; i<N; i++) {
  		pthread_join(ts[i], NULL);
  	}
  
  	return 0;
}

/* Implemented under the RA semantics
 * This benchmark is part of TRACER
 */

// Correia and Ramalhete CRHandover, Mutual Exclusion - Two linear wait software solutions
// https://github.com/pramalhe/ConcurrencyFreaks/tree/master/papers/cralgorithm-2015.pdf
//
// Shared words      = N+1
// Number of states  = 3
// Starvation-Free   = yes, with N
// Minimum SC stores = 1 + 1
// Minimum SC loads  = N+2

#include <threads.h>
#include <stdatomic.h>
#include "librace.h"
#include "model-assert.h"

#define LOOP 5		
#define N 3


enum State { UNLOCKED, WAITING, LOCKED };
enum Handover { NOTMINE, MYTURN };

#define CACHE_LINE  64
#define PADRATIO    (CACHE_LINE/sizeof(atomic_int))


// shared variables
atomic_int states[N*PADRATIO];
atomic_int handover[N*PADRATIO];
atomic_int hoEnabled;
atomic_int var;

atomic_int __fence_var;

void Worker(void *arg)
{
  	int id = *((int *)arg);
	int ok;
    int i, isFirstTime = 1;

    ok = 0;
    for (int j=0; j<LOOP; j++) {
        ok = 0;

        for (int jj=0; jj<LOOP; jj++) {
            if (!atomic_load_explicit(&hoEnabled, memory_order_acquire)) {
                ok = 1;
                break;
            }
            if (isFirstTime) {
                atomic_store_explicit(&states[id*PADRATIO], WAITING, memory_order_release); 
                isFirstTime = 0; 
            }
            if (atomic_load_explicit(&handover[id*PADRATIO], memory_order_acquire) == MYTURN) {
                atomic_store_explicit(&handover[id*PADRATIO], NOTMINE, memory_order_release);
                atomic_store_explicit(&states[id*PADRATIO], LOCKED, memory_order_release);
                goto LCS; // fast-path under high contention
            }
        }
        if (ok==0) return;

        isFirstTime = 0;
        atomic_store_explicit(&states[id*PADRATIO], LOCKED, memory_order_release);
        for (int jj = 0; jj < id; jj++) {
            if (atomic_load_explicit(&states[jj*PADRATIO], memory_order_acquire) != UNLOCKED) {
                i = j;
                break;
            }
        }

        if (i != id) {
            atomic_store_explicit(&states[id*PADRATIO], WAITING, memory_order_release);
            ok = 0;
            for (int jj=0; jj<LOOP; jj++) {
                if (atomic_load_explicit(&hoEnabled, memory_order_acquire)) {
                    ok = 1;
                    break;
                }

                for (int jjj = 0; jjj < id; jjj++) {
                    if (atomic_load_explicit(&states[jjj*PADRATIO], memory_order_acquire) != UNLOCKED) {
                        i = jjj;
                        break;
                    }
                }
                if (i == id) {
                    ok = 1;
                    break;
                }    
            }

            if (ok==0) return;    
            
            continue;
        }

        ok = 0;
        for (int jj=0; jj<LOOP; jj++) {
            if (atomic_load_explicit(&hoEnabled, memory_order_acquire)) {
                ok = 1;
                break;
            }
            for (int jjj = id + 1; jjj < N; jjj++) {
                if (atomic_load_explicit(&states[jjj*PADRATIO], memory_order_acquire) == LOCKED) {
                    i = jjj;
                    break;
                }
            }
            if (i == N && !atomic_load_explicit(&hoEnabled, memory_order_acquire)) {
                goto LCS;
            }
        }
        if (ok==0) return;
        atomic_store_explicit(&states[id*PADRATIO], WAITING, memory_order_release);
    }

    if (ok==0) return;

    LCS: // critical section
    atomic_store_explicit(&var, id, memory_order_release);
    MODEL_ASSERT(atomic_load_explicit(&var, memory_order_acquire)==id);

    for (int ii = id + 1; ii < N; ii++) {
        if (atomic_load_explicit(&states[ii*PADRATIO], memory_order_acquire) == WAITING) {
            if (!atomic_load_explicit(&hoEnabled, memory_order_acquire)) 
                atomic_store_explicit(&hoEnabled, 1, memory_order_release);
            atomic_store_explicit(&handover[ii*PADRATIO], MYTURN, memory_order_release);
            atomic_store_explicit(&states[id*PADRATIO], UNLOCKED, memory_order_release);
            goto LEND;
        }
    }

    for (int ii = 0; ii < id; ii++) {
        if (atomic_load_explicit(&states[ii*PADRATIO], memory_order_acquire) == WAITING) {
            if (!atomic_load_explicit(&hoEnabled, memory_order_acquire)) 
                atomic_store_explicit(&hoEnabled, 1, memory_order_release);
            atomic_store_explicit(&handover[ii*PADRATIO], MYTURN, memory_order_release);
            atomic_store_explicit(&states[id*PADRATIO], UNLOCKED, memory_order_release);
            goto LEND;
        }
    }

    // There are no successors at all
    if (atomic_load_explicit(&hoEnabled, memory_order_acquire)) 
        atomic_store_explicit(&hoEnabled, 0, memory_order_release);
    atomic_store_explicit(&states[id*PADRATIO], UNLOCKED, memory_order_release);
    LEND:

	return;

} // Worker


int user_main(int argc, char **argv)
{
  	thrd_t ts[N];
  	int arg[N];

	for (int i=0; i<N*PADRATIO; i++) {
	  	atomic_init(&states[i], UNLOCKED);
        atomic_init(&handover[i], NOTMINE);
	}
	atomic_init(&hoEnabled, 0);
	atomic_init(&var, 0);
	  
	for (int i=0; i<N; i++) {
	  	arg[i] = i;
	  	thrd_create(&ts[i], Worker, &arg[i]);
	}
  
  	for (int i=0; i<N; i++) {
  		thrd_join(ts[i]);
  	}
  
  	return 0;
}
/* Copyright (C) 2018 Tuan Phong Ngo 
 * This benchmark is part of TRACER
 */

/* Adapted from: https://svn.sosy-lab.org/software/sv-benchmarks/trunk/c/pthread-atomic/lamport_true-unreach-call.c */

#include <threads.h>
#include <stdatomic.h>
#include "librace.h"
#include "model-assert.h"

#define LOOP 4
#define OUTER_LOOP 2
#define LB LOOP * OUTER_LOOP

// shared variables
atomic_int x, y; 
atomic_int b1, b2;

atomic_int var;

atomic_int __fence_var;


void p1(void *arg)
{
  int ok;
  for (int jj=0; jj<OUTER_LOOP; jj++) {
    ok = 0;

    for (int i=0; i<LOOP; i++) {
      atomic_store_explicit(&b1, 1, memory_order_release);
      atomic_store_explicit(&x, 1, memory_order_release);

      atomic_fetch_add_explicit(&__fence_var, 0, memory_order_acq_rel);

      if (atomic_load_explicit(&y, memory_order_acquire) != 0) {
        atomic_store_explicit(&b1, 0, memory_order_release);

        for (int i=0; i<LOOP; i++) {
          if (atomic_load_explicit(&y, memory_order_acquire) == 0)
            goto breaklbl0;
        }

        goto breaklbl; 
        breaklbl0:;
        continue;
      }
      atomic_store_explicit(&y, 1, memory_order_release);

      atomic_fetch_add_explicit(&__fence_var, 0, memory_order_acq_rel);

      if (atomic_load_explicit(&x, memory_order_acquire) != 1) {
        atomic_store_explicit(&b1, 0, memory_order_release);

        for (int i=0; i<LOOP; i++) {
          if (atomic_load_explicit(&b2, memory_order_acquire) < 1)
            goto breaklbl1;
        }

        goto breaklbl; 
        breaklbl1:;

        if (atomic_load_explicit(&y, memory_order_acquire) != 1) {
          for (int i=0; i<LOOP; i++) {
            if (atomic_load_explicit(&y, memory_order_acquire) == 0)
              goto breaklbl2;
          }
          goto breaklbl; 
          breaklbl2:;
          continue;
        }
      }
      ok = 1;
      goto breaklbl;
    }

    breaklbl:;
    if (ok==0) return;

        // critical section
    atomic_store_explicit(&var, 1, memory_order_release);
    MODEL_ASSERT(atomic_load_explicit(&var, memory_order_acquire)==1);

    atomic_store_explicit(&y, 0, memory_order_release);
    atomic_store_explicit(&b1, 0, memory_order_release);
  }

}

void p2(void *arg)
{
  int ok;
  for (int jj=0; jj<OUTER_LOOP; jj++) {
    ok = 0;

    for (int i=0; i<LOOP; i++) {
      atomic_store_explicit(&b2, 1, memory_order_release);
      atomic_store_explicit(&x, 2, memory_order_release);

      atomic_fetch_add_explicit(&__fence_var, 0, memory_order_acq_rel);

      if (atomic_load_explicit(&y, memory_order_acquire) != 0) {
        atomic_store_explicit(&b2, 0, memory_order_release);

        for (int i=0; i<LOOP; i++) {
          if (atomic_load_explicit(&y, memory_order_acquire) == 0)
            goto breaklbl0;
        }

        goto breaklbl; 
        breaklbl0:;
        continue;

      }
      atomic_store_explicit(&y, 2, memory_order_release);

      atomic_fetch_add_explicit(&__fence_var, 0, memory_order_acq_rel);

      if (atomic_load_explicit(&x, memory_order_acquire) != 2) {
        atomic_store_explicit(&b2, 0, memory_order_release);

        for (int i=0; i<LOOP; i++) {
          if (atomic_load_explicit(&b1, memory_order_acquire) < 1)
            goto breaklbl1;
        }

        goto breaklbl; 
        breaklbl1:;

        if (atomic_load_explicit(&y, memory_order_acquire) != 2) {
          for (int i=0; i<LOOP; i++) {
            if (atomic_load_explicit(&y, memory_order_acquire) == 0)
              goto breaklbl2;
          }
          goto breaklbl; 
          breaklbl2:;
          continue;
        }
      }

      ok = 1;
      goto breaklbl;
    }

    breaklbl:;
    if (ok==0) return;

        // critical section
    atomic_store_explicit(&var, 2, memory_order_release);
    MODEL_ASSERT(atomic_load_explicit(&var, memory_order_acquire)==2);

    atomic_store_explicit(&y, 0, memory_order_release);
    atomic_store_explicit(&b2, 0, memory_order_release);
  }
}

int user_main(int argc, char **argv)
{
  thrd_t a, b;

  atomic_init(&x, 0);
  atomic_init(&y, 0);
  atomic_init(&b1, 0);
  atomic_init(&b2, 0);
  atomic_init(&var, 0);
  atomic_init(&__fence_var, 0);

  thrd_create(&a, p1, NULL);
  thrd_create(&b, p2, NULL);

  thrd_join(a);
  thrd_join(b);
  return 0;
}
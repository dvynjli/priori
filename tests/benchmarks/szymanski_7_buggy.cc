#include "../verify.h"

#define LOOP 4
#define OUTER_LOOP 2
#define LB LOOP*OUTER_LOOP

// shared variables
atomic<int> flag0; 
atomic<int> flag1;
atomic<int> flag2;
atomic<int> flag3;
atomic<int> flag4;
atomic<int> flag5;
atomic<int> flag6;

atomic<int> _cc_x;

atomic<int> __fence_var;

void *p0(void *arg)
{
	flag0.store(1, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	int rflag1 = flag1.load(memory_order_acquire);
	int rflag2 = flag2.load(memory_order_acquire);
	int rflag3 = flag3.load(memory_order_acquire);
	int rflag4 = flag4.load(memory_order_acquire);
	int rflag5 = flag5.load(memory_order_acquire);
	int rflag6 = flag6.load(memory_order_acquire);
	assume(rflag1<4 && rflag2<4 && rflag3<4 && rflag4<4 && rflag5<4 && rflag6<4);
	flag0.store(4, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	rflag1 = flag1.load(memory_order_acquire);
	rflag2 = flag2.load(memory_order_acquire);
	rflag3 = flag3.load(memory_order_acquire);
	rflag4 = flag4.load(memory_order_acquire);
	rflag5 = flag5.load(memory_order_acquire);
	rflag6 = flag6.load(memory_order_acquire);
	if(rflag1==1 || rflag2==1 || rflag3==1 || rflag4==1 || rflag5==1 || rflag6==1) {
		flag0.store(3, memory_order_release);
		__fence_var.fetch_add(0, memory_order_acq_rel);
		rflag1 = flag1.load(memory_order_acquire);
		rflag2 = flag2.load(memory_order_acquire);
		rflag3 = flag3.load(memory_order_acquire);
		rflag4 = flag4.load(memory_order_acquire);
		rflag5 = flag5.load(memory_order_acquire);
		rflag6 = flag6.load(memory_order_acquire);
		assume(rflag1==5 || rflag2==5 || rflag3==5 || rflag4==5 || rflag5==5 || rflag6==5);
	}
	flag0.store(5, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	
	// Critical Section
	_cc_x.store(0, memory_order_release);
	int rx = _cc_x.load(memory_order_acquire);
	assert(rx<=0);

	rflag1 = flag1.load(memory_order_acquire);
	rflag2 = flag2.load(memory_order_acquire);
	rflag3 = flag3.load(memory_order_acquire);
	rflag4 = flag4.load(memory_order_acquire);
	rflag5 = flag5.load(memory_order_acquire);
	rflag6 = flag6.load(memory_order_acquire);
	assume(rflag1 != 4 && rflag1 != 3 && rflag2 != 4 && rflag2 != 3 && rflag3 != 4 && rflag3 != 3 && rflag4 != 4 && rflag4 != 3 && rflag5 != 4 && rflag5 != 3 && rflag6 != 4 && rflag6 != 3 );
	flag0.store(0, memory_order_release);

	return NULL;
}

void *p1(void *arg)
{
		flag1.store(1, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	int rflag0 = flag0.load(memory_order_acquire);
	int rflag2 = flag2.load(memory_order_acquire);
	int rflag3 = flag3.load(memory_order_acquire);
	int rflag4 = flag4.load(memory_order_acquire);
	int rflag5 = flag5.load(memory_order_acquire);
	int rflag6 = flag6.load(memory_order_acquire);
	assume(rflag0<4 && rflag2<4 && rflag3<4 && rflag4<4 && rflag5<4 && rflag6<4);
	flag1.store(4, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	rflag0 = flag0.load(memory_order_acquire);
	rflag2 = flag2.load(memory_order_acquire);
	rflag3 = flag3.load(memory_order_acquire);
	rflag4 = flag4.load(memory_order_acquire);
	rflag5 = flag5.load(memory_order_acquire);
	rflag6 = flag6.load(memory_order_acquire);
	if(rflag0==1 || rflag2==1 || rflag3==1 || rflag4==1 || rflag5==1 || rflag6==1) {
		flag1.store(3, memory_order_release);
		__fence_var.fetch_add(0, memory_order_acq_rel);
		rflag0 = flag0.load(memory_order_acquire);
		rflag2 = flag2.load(memory_order_acquire);
		rflag3 = flag3.load(memory_order_acquire);
		rflag4 = flag4.load(memory_order_acquire);
		rflag5 = flag5.load(memory_order_acquire);
		rflag6 = flag6.load(memory_order_acquire);
		assume(rflag0==5 || rflag2==5 || rflag3==5 || rflag4==5 || rflag5==5 || rflag6==5);
	}
	flag1.store(5, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	rflag0 = flag0.load(memory_order_acquire);
	assume(rflag0 < 3);

	// Critical Section
	_cc_x.store(1, memory_order_release);
	int rx = _cc_x.load(memory_order_acquire);
	assert(rx==1);

	rflag2 = flag2.load(memory_order_acquire);
	rflag3 = flag3.load(memory_order_acquire);
	rflag4 = flag4.load(memory_order_acquire);
	rflag5 = flag5.load(memory_order_acquire);
	rflag6 = flag6.load(memory_order_acquire);
	assume(rflag2 != 4 && rflag2 != 3 && rflag3 != 4 && rflag3 != 3 && rflag4 != 4 && rflag4 != 3 && rflag5 != 4 && rflag5 != 3 && rflag6 != 4 && rflag6 != 3 );
	flag1.store(0, memory_order_release);

	return NULL;
}

void *p2(void *arg) {
		flag2.store(1, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	int rflag0 = flag0.load(memory_order_acquire);
	int rflag1 = flag1.load(memory_order_acquire);
	int rflag3 = flag3.load(memory_order_acquire);
	int rflag4 = flag4.load(memory_order_acquire);
	int rflag5 = flag5.load(memory_order_acquire);
	int rflag6 = flag6.load(memory_order_acquire);
	assume(rflag0<4 && rflag1<4 && rflag3<4 && rflag4<4 && rflag5<4 && rflag6<4);
	flag2.store(4, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	rflag0 = flag0.load(memory_order_acquire);
	rflag1 = flag1.load(memory_order_acquire);
	rflag3 = flag3.load(memory_order_acquire);
	rflag4 = flag4.load(memory_order_acquire);
	rflag5 = flag5.load(memory_order_acquire);
	rflag6 = flag6.load(memory_order_acquire);
	if(rflag0==1 || rflag1==1 || rflag3==1 || rflag4==1 || rflag5==1 || rflag6==1) {
		flag2.store(3, memory_order_release);
		__fence_var.fetch_add(0, memory_order_acq_rel);
		rflag0 = flag0.load(memory_order_acquire);
		rflag1 = flag1.load(memory_order_acquire);
		rflag3 = flag3.load(memory_order_acquire);
		rflag4 = flag4.load(memory_order_acquire);
		rflag5 = flag5.load(memory_order_acquire);
		rflag6 = flag6.load(memory_order_acquire);
		assume(rflag0==5 || rflag1==5 || rflag3==5 || rflag4==5 || rflag5==5 || rflag6==5);
	}
	flag2.store(5, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	rflag0 = flag0.load(memory_order_acquire);
	rflag1 = flag1.load(memory_order_acquire);
	assume(rflag0 < 3 && rflag1 < 3);

	// Critical Section
	_cc_x.store(1, memory_order_release);
	int rx = _cc_x.load(memory_order_acquire);
	assert(rx==2);

	rflag3 = flag3.load(memory_order_acquire);
	rflag4 = flag4.load(memory_order_acquire);
	rflag5 = flag5.load(memory_order_acquire);
	rflag6 = flag6.load(memory_order_acquire);
	assume(rflag3 != 4 && rflag3 != 3 && rflag4 != 4 && rflag4 != 3 && rflag5 != 4 && rflag5 != 3 && rflag6 != 4 && rflag6 != 3 );
	flag2.store(0, memory_order_release);
	
	return NULL;
}

void *p3(void *arg) {
		flag3.store(1, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	int rflag0 = flag0.load(memory_order_acquire);
	int rflag1 = flag1.load(memory_order_acquire);
	int rflag2 = flag2.load(memory_order_acquire);
	int rflag4 = flag4.load(memory_order_acquire);
	int rflag5 = flag5.load(memory_order_acquire);
	int rflag6 = flag6.load(memory_order_acquire);
	assume(rflag0<4 && rflag1<4 && rflag2<4 && rflag4<4 && rflag5<4 && rflag6<4);
	flag3.store(4, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	rflag0 = flag0.load(memory_order_acquire);
	rflag1 = flag1.load(memory_order_acquire);
	rflag2 = flag2.load(memory_order_acquire);
	rflag4 = flag4.load(memory_order_acquire);
	rflag5 = flag5.load(memory_order_acquire);
	rflag6 = flag6.load(memory_order_acquire);
	if(rflag0==1 || rflag1==1 || rflag2==1 || rflag4==1 || rflag5==1 || rflag6==1) {
		flag3.store(3, memory_order_release);
		__fence_var.fetch_add(0, memory_order_acq_rel);
		rflag0 = flag0.load(memory_order_acquire);
		rflag1 = flag1.load(memory_order_acquire);
		rflag2 = flag2.load(memory_order_acquire);
		rflag4 = flag4.load(memory_order_acquire);
		rflag5 = flag5.load(memory_order_acquire);
		rflag6 = flag6.load(memory_order_acquire);
		assume(rflag0==5 || rflag1==5 || rflag2==5 || rflag4==5 || rflag5==5 || rflag6==5);
	}
	flag3.store(5, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	rflag0 = flag0.load(memory_order_acquire);
	rflag1 = flag1.load(memory_order_acquire);
	rflag2 = flag2.load(memory_order_acquire);
	assume(rflag0 < 3 && rflag1 < 3 && rflag2 < 3);

	// Critical Section
	_cc_x.store(3, memory_order_release);
	int rx = _cc_x.load(memory_order_acquire);
	assert(rx==3);

	rflag4 = flag4.load(memory_order_acquire);
	rflag5 = flag5.load(memory_order_acquire);
	rflag6 = flag6.load(memory_order_acquire);
	assume(rflag4 != 4 && rflag4 != 3 && rflag5 != 4 && rflag5 != 3 && rflag6 != 4 && rflag6 != 3 );
	flag3.store(0, memory_order_release);
	
	return NULL;
}

void *p4(void *arg) {
		flag4.store(1, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	int rflag0 = flag0.load(memory_order_acquire);
	int rflag1 = flag1.load(memory_order_acquire);
	int rflag2 = flag2.load(memory_order_acquire);
	int rflag3 = flag3.load(memory_order_acquire);
	int rflag5 = flag5.load(memory_order_acquire);
	int rflag6 = flag6.load(memory_order_acquire);
	assume(rflag0<4 && rflag1<4 && rflag2<4 && rflag3<4 && rflag5<4 && rflag6<4);
	flag4.store(4, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	rflag0 = flag0.load(memory_order_acquire);
	rflag1 = flag1.load(memory_order_acquire);
	rflag2 = flag2.load(memory_order_acquire);
	rflag3 = flag3.load(memory_order_acquire);
	rflag5 = flag5.load(memory_order_acquire);
	rflag6 = flag6.load(memory_order_acquire);
	if(rflag0==1 || rflag1==1 || rflag2==1 || rflag3==1 || rflag5==1 || rflag6==1) {
		flag4.store(3, memory_order_release);
		__fence_var.fetch_add(0, memory_order_acq_rel);
		rflag0 = flag0.load(memory_order_acquire);
		rflag1 = flag1.load(memory_order_acquire);
		rflag2 = flag2.load(memory_order_acquire);
		rflag3 = flag3.load(memory_order_acquire);
		rflag5 = flag5.load(memory_order_acquire);
		rflag6 = flag6.load(memory_order_acquire);
		assume(rflag0==5 || rflag1==5 || rflag2==5 || rflag3==5 || rflag5==5 || rflag6==5);
	}
	flag4.store(5, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	rflag0 = flag0.load(memory_order_acquire);
	rflag1 = flag1.load(memory_order_acquire);
	rflag2 = flag2.load(memory_order_acquire);
	rflag3 = flag3.load(memory_order_acquire);
	assume(rflag0 < 3 && rflag1 < 3 && rflag2 < 3 && rflag3 < 3);

	// Critical Section
	_cc_x.store(4, memory_order_release);
	int rx = _cc_x.load(memory_order_acquire);
	assert(rx==4);

	rflag5 = flag5.load(memory_order_acquire);
	rflag6 = flag6.load(memory_order_acquire);
	assume(rflag5 != 4 && rflag5 != 3 && rflag6 != 4 && rflag6 != 3 );
	flag4.store(0, memory_order_release);

	return NULL;
}

void *p5(void *arg) {
		flag5.store(1, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	int rflag0 = flag0.load(memory_order_acquire);
	int rflag1 = flag1.load(memory_order_acquire);
	int rflag2 = flag2.load(memory_order_acquire);
	int rflag3 = flag3.load(memory_order_acquire);
	int rflag4 = flag4.load(memory_order_acquire);
	int rflag6 = flag6.load(memory_order_acquire);
	assume(rflag0<4 && rflag1<4 && rflag2<4 && rflag3<4 && rflag4<4 && rflag6<4);
	flag5.store(4, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	rflag0 = flag0.load(memory_order_acquire);
	rflag1 = flag1.load(memory_order_acquire);
	rflag2 = flag2.load(memory_order_acquire);
	rflag3 = flag3.load(memory_order_acquire);
	rflag4 = flag4.load(memory_order_acquire);
	rflag6 = flag6.load(memory_order_acquire);
	if(rflag0==1 || rflag1==1 || rflag2==1 || rflag3==1 || rflag4==1 || rflag6==1) {
		flag5.store(3, memory_order_release);
		__fence_var.fetch_add(0, memory_order_acq_rel);
		rflag0 = flag0.load(memory_order_acquire);
		rflag1 = flag1.load(memory_order_acquire);
		rflag2 = flag2.load(memory_order_acquire);
		rflag3 = flag3.load(memory_order_acquire);
		rflag4 = flag4.load(memory_order_acquire);
		rflag6 = flag6.load(memory_order_acquire);
		assume(rflag0==5 || rflag1==5 || rflag2==5 || rflag3==5 || rflag4==5 || rflag6==5);
	}
	flag5.store(5, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	rflag0 = flag0.load(memory_order_acquire);
	rflag1 = flag1.load(memory_order_acquire);
	rflag2 = flag2.load(memory_order_acquire);
	rflag3 = flag3.load(memory_order_acquire);
	rflag4 = flag4.load(memory_order_acquire);
	assume(rflag0 < 3 && rflag1 < 3 && rflag2 < 3 && rflag3 < 3 && rflag4 < 3);

	// Critical Section
	_cc_x.store(5, memory_order_release);
	int rx = _cc_x.load(memory_order_acquire);
	assert(rx==5);

	rflag6 = flag6.load(memory_order_acquire);
	assume(rflag6 != 4 && rflag6 != 3 );
	flag5.store(0, memory_order_release);
	
	return NULL;
}

void *p6(void *arg) {
		flag6.store(1, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	int rflag0 = flag0.load(memory_order_acquire);
	int rflag1 = flag1.load(memory_order_acquire);
	int rflag2 = flag2.load(memory_order_acquire);
	int rflag3 = flag3.load(memory_order_acquire);
	int rflag4 = flag4.load(memory_order_acquire);
	int rflag5 = flag5.load(memory_order_acquire);
	assume(rflag0<4 && rflag1<4 && rflag2<4 && rflag3<4 && rflag4<4 && rflag5<4);
	flag6.store(4, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	rflag0 = flag0.load(memory_order_acquire);
	rflag1 = flag1.load(memory_order_acquire);
	rflag2 = flag2.load(memory_order_acquire);
	rflag3 = flag3.load(memory_order_acquire);
	rflag4 = flag4.load(memory_order_acquire);
	rflag5 = flag5.load(memory_order_acquire);
	if(rflag0==1 || rflag1==1 || rflag2==1 || rflag3==1 || rflag4==1 || rflag5==1) {
		flag6.store(3, memory_order_release);
		__fence_var.fetch_add(0, memory_order_acq_rel);
		rflag0 = flag0.load(memory_order_acquire);
		rflag1 = flag1.load(memory_order_acquire);
		rflag2 = flag2.load(memory_order_acquire);
		rflag3 = flag3.load(memory_order_acquire);
		rflag4 = flag4.load(memory_order_acquire);
		rflag5 = flag5.load(memory_order_acquire);
		assume(rflag0==5 || rflag1==5 || rflag2==5 || rflag3==5 || rflag4==5 || rflag5==5);
	}
	flag6.store(5, memory_order_release);
	__fence_var.fetch_add(0, memory_order_acq_rel);
	rflag0 = flag0.load(memory_order_acquire);
	rflag1 = flag1.load(memory_order_acquire);
	rflag2 = flag2.load(memory_order_acquire);
	rflag3 = flag3.load(memory_order_acquire);
	rflag4 = flag4.load(memory_order_acquire);
	rflag5 = flag5.load(memory_order_acquire);
	assume(rflag0 < 3 && rflag1 < 3 && rflag2 < 3 && rflag3 < 3 && rflag4 < 3 && rflag5 < 3);

	// Critical Section
	_cc_x.store(6, memory_order_release);
	int rx = _cc_x.load(memory_order_acquire);
	assert(rx==6);

	flag6.store(0, memory_order_release);

	
	return NULL;
}

int main()
{
	pthread_t t0, t1, t2, t3, t4, t5, t6;

  pthread_create(&t0, NULL, p0, NULL);
  pthread_create(&t1, NULL, p1, NULL);
  pthread_create(&t2, NULL, p2, NULL);
  pthread_create(&t3, NULL, p3, NULL);
  pthread_create(&t4, NULL, p4, NULL);
  pthread_create(&t5, NULL, p5, NULL);
  pthread_create(&t6, NULL, p6, NULL);

  pthread_join(t0, NULL);
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  pthread_join(t3, NULL);
  pthread_join(t4, NULL);
  pthread_join(t5, NULL);
  pthread_join(t6, NULL);

 return 0;
}

#include "../verify.h"

atomic<int> level0, level1, level2;
atomic<int> waiting0, waiting1;

atomic<int> _cc_x; // to avoid race
atomic<int> __fence_var;


void *p0(void *arg)
{
	int rwaiting = -1, rlevel1 = -1, rlevel2 = -1, rx = -1;
	level0.store(0, REL);
	__fence_var.fetch_add(0, ACQREL);
	waiting0.store(0, REL);
	__fence_var.fetch_add(0, ACQREL);
	rwaiting = waiting0.load(ACQ);
	rlevel1 = level1.load(ACQ);
	rlevel2 = level2.load(ACQ);
	assume(rwaiting != 0);
	level0.store(1, REL);
	__fence_var.fetch_add(0, ACQREL);
	waiting1.store(0, REL);
	__fence_var.fetch_add(0, ACQREL);

	rwaiting = waiting1.load(ACQ);
	rlevel1 = level1.load(ACQ);
	rlevel2 = level2.load(ACQ);
	assume(rwaiting != 0 || (rlevel1 < 1 && rlevel2 < 1));
	_cc_x.store(0, REL);
	rx = _cc_x.load(ACQ);
	assert(rx==0);
	__fence_var.fetch_add(0, ACQREL);
	level0.store(0, REL);
	
	return NULL;
}

void *p1(void *arg)
{
	int rwaiting = -1, rlevel0 = -1, rlevel2 = -1, rx = -1;
	level1.store(0, REL);
	__fence_var.fetch_add(0, ACQREL);
	waiting0.store(1, REL);
	__fence_var.fetch_add(0, ACQREL);

	rwaiting = waiting0.load(ACQ);
	rlevel2 = level2.load(ACQ);
	rlevel0 = level0.load(ACQ);
	// assume(rwaiting != 1 || (rlevel2 < 0 && rlevel0 < 0));
	assume(rwaiting != 1);
	level1.store(1, REL);
	__fence_var.fetch_add(0, ACQREL);
	waiting1.store(1, REL);
	__fence_var.fetch_add(0, ACQREL);

	rwaiting = waiting1.load(ACQ);
	rlevel2 = level2.load(ACQ);
	rlevel0 = level0.load(ACQ);
	assume(rwaiting != 1 || (rlevel2 < 1 && rlevel0 < 1));
	_cc_x.store(1, REL);
	rx = _cc_x.load(ACQ);
	assert(rx==1);
	__fence_var.fetch_add(0, ACQREL);
	level1.store(0, REL);

	return NULL;
}	

void *p2(void *arg) {
	int rwaiting = -1, rlevel0 = -1, rlevel1 = -1, rx = -1;
	level2.store(0, REL);
	// __fence_var.fetch_add(0, ACQREL);
	waiting0.store(2, REL);
	// __fence_var.fetch_add(0, ACQREL);

	rwaiting = waiting0.load(ACQ);
	rlevel1 = level1.load(ACQ);
	rlevel0 = level0.load(ACQ);
	assume(rwaiting != 2);
	level2.store(1, REL);
	// __fence_var.fetch_add(0, ACQREL);
	waiting1.store(2, REL);
	// __fence_var.fetch_add(0, ACQREL);

	rwaiting = waiting1.load(ACQ);
	rlevel1 = level1.load(ACQ);
	rlevel0 = level0.load(ACQ);
	assume(rwaiting != 2 || (rlevel1 < 1 && rlevel0 < 1));
	_cc_x.store(2, REL);
	rx = _cc_x.load(ACQ);
	assert(rx==2);
	// __fence_var.fetch_add(0, ACQREL);
	level2.store(0, REL);
	return NULL;
}

int main()
{
 level0.store(0, REL);
 level1.store(0, REL);
 level2.store(0, REL);
 waiting0.store(0, REL);
 waiting1.store(0, REL);
 __fence_var.store(0, REL);
 pthread_t a, b, c;

 pthread_create(&a, NULL, p0, NULL);
 pthread_create(&b, NULL, p1, NULL);
 pthread_create(&c, NULL, p2, NULL);

 pthread_join(a, NULL);
 pthread_join(b, NULL);
 pthread_join(c, NULL);

 return 0;
}

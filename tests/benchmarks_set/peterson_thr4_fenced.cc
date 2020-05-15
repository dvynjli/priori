#include "../verify.h"

atomic<int> level0, level1, level2, level3;
atomic<int> waiting0, waiting1, waiting2;
atomic<int> _cc_x;
atomic<int> __fence_var;
void* t0(void *){
	int rwaiting0=-1, rwaiting1=-1, rwaiting2=-1, rlevel1=-1, rlevel2=-1, rlevel3=-1, rx=-1;
	level0.store(0,REL);
	// __fence_var.fetch_add(0, ACQREL);
	waiting0.store(0, REL);
	// __fence_var.fetch_add(0, ACQREL);
	rwaiting0 = waiting0.load(ACQ);
	assume(rwaiting0!=0);
	level0.store(1,REL);
	// __fence_var.fetch_add(0, ACQREL);
	waiting1.store(0,REL);
	// __fence_var.fetch_add(0, ACQREL);
	rwaiting1 = waiting1.load(ACQ);
	rlevel1 = level1.load(ACQ);
	rlevel2 = level2.load(ACQ);
	rlevel3 = level3.load(ACQ);
	assume(rwaiting1!=0 || (rlevel1 < 1 && rlevel2 < 1 && rlevel3 < 1));
	level0.store(2, REL);
	// __fence_var.fetch_add(0, ACQREL);
	waiting2.store(0,REL);
	// __fence_var.fetch_add(0, ACQREL);
	rwaiting2 = waiting2.load(ACQ);
	rlevel1 = level1.load(ACQ);
	rlevel2 = level2.load(ACQ);
	rlevel3 = level3.load(ACQ);
	assume(rwaiting2!=0 || (rlevel1 < 2 && rlevel2 < 2 && rlevel3 < 2));
	
	_cc_x.store(0,REL);
	rx = _cc_x.load(ACQ);
	assert(rx==0);
	// __fence_var.fetch_add(0, ACQREL);
	level0.store(0,REL);
	return NULL;
}

void* t1(void *){
	int rwaiting0=-1, rwaiting1=-1, rwaiting2=-1, rlevel0=-1, rlevel2=-1, rlevel3=-1, rx=-1;
	level1.store(0, REL);
	// __fence_var.fetch_add(0, ACQREL);
	waiting0.store(1,REL);
	// __fence_var.fetch_add(0, ACQREL);
	rwaiting0 = waiting0.load(ACQ);
	assume(rwaiting0!=1);
	level1.store(1, REL);
	// __fence_var.fetch_add(0, ACQREL);
	waiting1.store(1,REL);
	// __fence_var.fetch_add(0, ACQREL);
	rwaiting1 = waiting1.load(ACQ);
	rlevel0 = level0.load(ACQ);
	rlevel2 = level2.load(ACQ);
	rlevel3 = level3.load(ACQ);
	assume(rwaiting1!=1 || (rlevel0 < 1 && rlevel2 < 1 && rlevel3 < 1));
	level1.store(2, REL);
	// __fence_var.fetch_add(0, ACQREL);
	waiting2.store(1, REL);
	// __fence_var.fetch_add(0, ACQREL);
	rwaiting2 = waiting2.load(ACQ);
	rlevel0 = level0.load(ACQ);
	rlevel2 = level2.load(ACQ);
	rlevel3 = level3.load(ACQ);
	assume(rwaiting2!=1 || (rlevel0 < 2 && rlevel2 < 2 && rlevel3 < 2));

	_cc_x.store(1, REL);
	rx = _cc_x.load(ACQ);
	assert(rx==1);
	// __fence_var.fetch_add(0, ACQREL);
	level1.store(0, REL);
	return NULL;
}

void* t2(void *){
	int rwaiting0=-1, rwaiting1=-1, rwaiting2=-1, rlevel0=-1, rlevel1=-1, rlevel3=-1, rx=-1;
	level2.store(0, REL);
	
	waiting0.store(2, REL);
	
	rwaiting0 = waiting0.load(ACQ);
	assume(rwaiting0!=2);
	level2.store(1, REL);
	
	waiting1.store(2, REL);
	
	rwaiting1 = waiting1.load(ACQ);
	rlevel0 = level0.load(ACQ);
	rlevel1 = level1.load(ACQ);
	rlevel3 = level3.load(ACQ);
	assume(rwaiting1!=2 || (rlevel0 < 1 && rlevel1 < 1 && rlevel3 < 1));
	level2.store(2, REL);
	
	waiting2.store(2, REL);

	rwaiting2 = waiting2.load(ACQ);
	rlevel0 = level0.load(ACQ);
	rlevel1 = level1.load(ACQ);
	rlevel3 = level3.load(ACQ);
	assume(rwaiting2!=2 || (rlevel0 < 2 && rlevel1 < 2 && rlevel3 < 2));
	_cc_x.store(2, REL);
	rx = _cc_x.load(ACQ);
	assert(rx==2);
	
	level2.store(0, REL);
	return NULL;
}
void*t3(){
		while(1){
				level3 = 0;
		__fence_var.fetch_add(0, ACQREL);
		waiting0=3;
		__fence_var.fetch_add(0, ACQREL);
		assume(waiting0!=3 || (level0 < 0 && level1 < 0 && level2 < 0 && 1));
		level3 = 1;
		__fence_var.fetch_add(0, ACQREL);
		waiting1=3;
		__fence_var.fetch_add(0, ACQREL);
		assume(waiting1!=3 || (level0 < 1 && level1 < 1 && level2 < 1 && 1));
		level3 = 2;
		__fence_var.fetch_add(0, ACQREL);
		waiting2=3;
		__fence_var.fetch_add(0, ACQREL);
		assume(waiting2!=3 || (level0 < 2 && level1 < 2 && level2 < 2 && 1));
		_cc_x = 3;
		assert(_cc_x==3);
		__fence_var.fetch_add(0, ACQREL);
		level3 = 0;
		}
}
int main(){
	pthread_t _cs_tdiff0, _cs_tdiff1, _cs_tdiff2, _cs_tdiff3;
	pthread_create(&_cs_tdiff0, 0, t0, 0);
	pthread_create(&_cs_tdiff1, 0, t1, 0);
	pthread_create(&_cs_tdiff2, 0, t2, 0);
	// pthread_create(&_cs_tdiff3, 0, t3, 0);
	pthread_join(_cs_tdiff0, NULL);
 	pthread_join(_cs_tdiff1, NULL);
 	pthread_join(_cs_tdiff2, NULL);
	return 0;
	}


#include "../verify.h"

#define FAA(var_name, value) var_name.fetch_add(value, ACQREL);

atomic<int> level0, level1, level2, level3;
atomic<int> waiting0, waiting1, waiting2;
atomic<int> _cc_x;
atomic<int> __fence_var;

void*t0(void *arg){
	int rwaiting = -1, rlevel1 = -1, rlevel2 = -1, rlevel3=-1, rx = -1;
	//while(1){
	level0.store(0, REL);
	FAA(__fence_var, 0);
	waiting0.store(0, REL);
	FAA(__fence_var, 0);
	rwaiting = waiting0.load(ACQ);
	rlevel1 = level1.load(ACQ);
	rlevel2 = level2.load(ACQ);
	rlevel3 = level3.load(ACQ);
	// assume(rwaiting!=0 || (rlevel1 < 0 && rlevel2 < 0 && rlevel3 < 0));
	assume(rwaiting!=0);
	level0.store(1,REL);
	FAA(__fence_var, 0);
	waiting1.store(0, REL);
	FAA(__fence_var, 0);
	rwaiting = waiting1.load(ACQ);
	rlevel1 = level1.load(ACQ);
	rlevel2 = level2.load(ACQ);
	rlevel3 = level3.load(ACQ);
	assume(rwaiting!=0 || (rlevel1 < 1 && rlevel2 < 1 && rlevel3 < 1));
	level0.store(2, REL);
	FAA(__fence_var, 0);
	waiting2.store(0, REL);
	FAA(__fence_var, 0);
	rwaiting = waiting2.load(ACQ);
	rlevel1 = level1.load(ACQ);
	rlevel2 = level2.load(ACQ);
	rlevel3 = level3.load(ACQ);
	assume(rwaiting!=0 || (rlevel1 < 2 && rlevel2 < 2 && rlevel3 < 2));
	_cc_x.store(0, REL);
	rx = _cc_x.load(ACQ);
	assert(rx==0);
	FAA(__fence_var, 0);
	level0.store(0, REL);
	return NULL;
	//}
}
void*t1(void *arg){
	int rwaiting = -1, rlevel0 = -1, rlevel2 = -1, rlevel3=-1, rx = -1;
	//while(1){
	level1.store(0, REL);
	FAA(__fence_var, 0);
	waiting0.store(1, REL);
	FAA(__fence_var, 0);
	rwaiting = waiting0.load(ACQ);
	rlevel0 = level0.load(ACQ);
	rlevel2 = level2.load(ACQ);
	rlevel3 = level3.load(ACQ);
	// assume(rwaiting!=1 || (rlevel0 < 0 && rlevel2 < 0 && rlevel3 < 0));
	assume(rwaiting!=1);
	level1.store(1,REL);
	FAA(__fence_var, 0);
	waiting1.store(1, REL);
	FAA(__fence_var, 0);
	rwaiting = waiting1.load(ACQ);
	rlevel0 = level0.load(ACQ);
	rlevel2 = level2.load(ACQ);
	rlevel3 = level3.load(ACQ);
	assume(rwaiting!=1 || (rlevel0 < 1 && rlevel2 < 1 && rlevel3 < 1));
	level1.store(2, REL);
	FAA(__fence_var, 0);
	waiting2.store(1, REL);
	FAA(__fence_var, 0);
	rwaiting = waiting2.load(ACQ);
	rlevel0 = level0.load(ACQ);
	rlevel2 = level2.load(ACQ);
	rlevel3 = level3.load(ACQ);
	assume(rwaiting!=1 || (rlevel0 < 2 && rlevel2 < 2 && rlevel3 < 2));
	_cc_x.store(1, REL);
	rx = _cc_x.load(ACQ);
	assert(rx==1);
	FAA(__fence_var, 0);
	level1.store(0, REL);
	return NULL;
	//}
}
void*t2(void *arg){
	int rwaiting = -1, rlevel0 = -1, rlevel1 = -1, rlevel3=-1, rx = -1;
	//while(1){
	level2.store(0, REL);
	// FAA(__fence_var, 0);
	waiting0.store(2, REL);
	// FAA(__fence_var, 0);
	rwaiting = waiting0.load(ACQ);
	rlevel0 = level0.load(ACQ);
	rlevel1 = level1.load(ACQ);
	rlevel3 = level3.load(ACQ);
	//assume(rwaiting!=2 || (rlevel0 < 0 && rlevel1 < 0 && rlevel3 < 0));
	assume(rwaiting!=2);
	level2.store(1,REL);
	// FAA(__fence_var, 0);
	waiting1.store(2, REL);
	// FAA(__fence_var, 0);
	rwaiting = waiting1.load(ACQ);
	rlevel0 = level0.load(ACQ);
	rlevel1 = level1.load(ACQ);
	rlevel3 = level3.load(ACQ);
	assume(rwaiting!=2 || (rlevel0 < 1 && rlevel1 < 1 && rlevel3 < 1));
	level2.store(2, REL);
	// FAA(__fence_var, 0);
	waiting2.store(2, REL);
	// FAA(__fence_var, 0);
	rwaiting = waiting2.load(ACQ);
	rlevel0 = level0.load(ACQ);
	rlevel1 = level1.load(ACQ);
	rlevel3 = level3.load(ACQ);
	assume(rwaiting!=2 || (rlevel0 < 2 && rlevel1 < 2 && rlevel3 < 2));
	_cc_x.store(2, REL);
	rx = _cc_x.load(ACQ);
	assert(rx==2);
	// FAA(__fence_var, 0);
	level2.store(0, REL);
	return NULL;
	//}
		//}
}
void*t3(void *arg){
	int rwaiting = -1, rlevel0 = -1, rlevel1 = -1, rlevel2=-1, rx = -1;
	//while(1){
	level3.store(0, REL);
	FAA(__fence_var, 0);
	waiting0.store(3, REL);
	FAA(__fence_var, 0);
	rwaiting = waiting0.load(ACQ);
	rlevel0 = level0.load(ACQ);
	rlevel1 = level1.load(ACQ);
	rlevel2 = level2.load(ACQ);
	// assume(rwaiting!=3 || (rlevel0 < 0 && rlevel1 < 0 && rlevel2 < 0));
	assume(rwaiting!=3);
	level3.store(1,REL);
	FAA(__fence_var, 0);
	waiting1.store(3, REL);
	FAA(__fence_var, 0);
	rwaiting = waiting1.load(ACQ);
	rlevel0 = level0.load(ACQ);
	rlevel1 = level1.load(ACQ);
	rlevel2 = level2.load(ACQ);
	assume(rwaiting!=3 || (rlevel0 < 1 && rlevel1 < 1 && rlevel2 < 1));
	level3.store(2, REL);
	FAA(__fence_var, 0);
	waiting2.store(3, REL);
	FAA(__fence_var, 0);
	rwaiting = waiting2.load(ACQ);
	rlevel0 = level0.load(ACQ);
	rlevel1 = level1.load(ACQ);
	rlevel2 = level2.load(ACQ);
	assume(rwaiting!=3 || (rlevel0 < 2 && rlevel1 < 2 && rlevel2 < 2));
	_cc_x.store(3, REL);
	rx = _cc_x.load(ACQ);
	assert(rx==3);
	FAA(__fence_var, 0);
	level3.store(0, REL);
	return NULL;
	//}
}
int main(){
	level0.store(0,REL);
	level1.store(0,REL);
	level2.store(0,REL);
	level3.store(0,REL);
	waiting0.store(0, REL);
	waiting1.store(0, REL);
	waiting2.store(0, REL);
	__fence_var.store(0, REL);
	

	pthread_t _cs_tdiff0, _cs_tdiff1, _cs_tdiff2, _cs_tdiff3;
	pthread_create(&_cs_tdiff0, NULL, t0, NULL);
	pthread_create(&_cs_tdiff1, NULL, t1, NULL);
	pthread_create(&_cs_tdiff2, NULL, t2, NULL);
	pthread_create(&_cs_tdiff3, NULL, t3, NULL);
	
	pthread_join(_cs_tdiff0, NULL);
	pthread_join(_cs_tdiff1, NULL);
	pthread_join(_cs_tdiff2, NULL);
	pthread_join(_cs_tdiff3, NULL);
	
	return 0;
	}


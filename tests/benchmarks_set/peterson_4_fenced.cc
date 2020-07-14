#include "../verify.h"


atomic<int> level0, level1, level2, level3;
atomic<int> waiting0, waiting1, waiting2;
atomic<int> _cc_x;
atomic<int> __fence_var;

void*t0(void *arg){
		//while(1){
		level0 = 0;
		rmw(__fence_var, 0);
		waiting0=0;
		rmw(__fence_var, 0);
		assume(waiting0!=0 || (level1 < 0 && level2 < 0 && level3 < 0 && 1));
		level0 = 1;
		rmw(__fence_var, 0);
		waiting1=0;
		rmw(__fence_var, 0);
		assume(waiting1!=0 || (level1 < 1 && level2 < 1 && level3 < 1 && 1));
		level0 = 2;
		rmw(__fence_var, 0);
		waiting2=0;
		rmw(__fence_var, 0);
		assume(waiting2!=0 || (level1 < 2 && level2 < 2 && level3 < 2 && 1));
		_cc_x = 0;
		assert(_cc_x==0);
		rmw(__fence_var, 0);
		level0 = 0;
		//}
}
void*t1(void *arg){
		//while(1){
		level1 = 0;
		rmw(__fence_var, 0);
		waiting0=1;
		rmw(__fence_var, 0);
		assume(waiting0!=1 || (level0 < 0 && level2 < 0 && level3 < 0 && 1));
		level1 = 1;
		rmw(__fence_var, 0);
		waiting1=1;
		rmw(__fence_var, 0);
		assume(waiting1!=1 || (level0 < 1 && level2 < 1 && level3 < 1 && 1));
		level1 = 2;
		rmw(__fence_var, 0);
		waiting2=1;
		rmw(__fence_var, 0);
		assume(waiting2!=1 || (level0 < 2 && level2 < 2 && level3 < 2 && 1));
		_cc_x = 1;
		assert(_cc_x==1);
		rmw(__fence_var, 0);
		level1 = 0;
		//}
}
void*t2(void *arg){
		//while(1){
				level2 = 0;
		
		waiting0=2;
		
		assume(waiting0!=2 || (level0 < 0 && level1 < 0 && level3 < 0 && 1));
		level2 = 1;
		
		waiting1=2;
		
		assume(waiting1!=2 || (level0 < 1 && level1 < 1 && level3 < 1 && 1));
		level2 = 2;
		
		waiting2=2;
		
		assume(waiting2!=2 || (level0 < 2 && level1 < 2 && level3 < 2 && 1));
		_cc_x = 2;
		assert(_cc_x==2);
		
		level2 = 0;
		//}
}
void*t3(void *arg){
		//while(1){
		level3 = 0;
		rmw(__fence_var, 0);
		waiting0=3;
		rmw(__fence_var, 0);
		assume(waiting0!=3 || (level0 < 0 && level1 < 0 && level2 < 0 && 1));
		level3 = 1;
		rmw(__fence_var, 0);
		waiting1=3;
		rmw(__fence_var, 0);
		assume(waiting1!=3 || (level0 < 1 && level1 < 1 && level2 < 1 && 1));
		level3 = 2;
		rmw(__fence_var, 0);
		waiting2=3;
		rmw(__fence_var, 0);
		assume(waiting2!=3 || (level0 < 2 && level1 < 2 && level2 < 2 && 1));
		_cc_x = 3;
		assert(_cc_x==3);
		rmw(__fence_var, 0);
		level3 = 0;
		//}
}
int main(){
	pthread_t _cs_tdiff0, _cs_tdiff1, _cs_tdiff2, _cs_tdiff3;
	pthread_create(&_cs_tdiff0, 0, t0, 0);
	pthread_create(&_cs_tdiff1, 0, t1, 0);
	pthread_create(&_cs_tdiff2, 0, t2, 0);
	pthread_create(&_cs_tdiff3, 0, t3, 0);
	return 0;
	}


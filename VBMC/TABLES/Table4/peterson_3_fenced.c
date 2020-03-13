#include <pthread.h>


int level0, level1, level2;
int waiting0, waiting1;
int _cc_x;
int __fence_var;
void*t0(){
		while(1){
				level0 = 0;
		rmw(__fence_var, 0);
		waiting0=0;
		rmw(__fence_var, 0);
		assume(waiting0!=0 || (level1 < 0 && level2 < 0 && 1));
		level0 = 1;
		rmw(__fence_var, 0);
		waiting1=0;
		rmw(__fence_var, 0);
		assume(waiting1!=0 || (level1 < 1 && level2 < 1 && 1));
		_cc_x = 0;
		assert(_cc_x==0);
		rmw(__fence_var, 0);
		level0 = 0;
		}
}
void*t1(){
		while(1){
				level1 = 0;
		rmw(__fence_var, 0);
		waiting0=1;
		rmw(__fence_var, 0);
		assume(waiting0!=1 || (level0 < 0 && level2 < 0 && 1));
		level1 = 1;
		rmw(__fence_var, 0);
		waiting1=1;
		rmw(__fence_var, 0);
		assume(waiting1!=1 || (level0 < 1 && level2 < 1 && 1));
		_cc_x = 1;
		assert(_cc_x==1);
		rmw(__fence_var, 0);
		level1 = 0;
		}
}
void*t2(){
		while(1){
				level2 = 0;
		rmw(__fence_var, 0);
		waiting0=2;
		rmw(__fence_var, 0);
		assume(waiting0!=2 || (level0 < 0 && level1 < 0 && 1));
		level2 = 1;
		rmw(__fence_var, 0);
		waiting1=2;
		rmw(__fence_var, 0);
		assume(waiting1!=2 || (level0 < 1 && level1 < 1 && 1));
		_cc_x = 1;
		assert(_cc_x==2);
		rmw(__fence_var, 0);
		level2 = 0;
		}
}
int main(){
	pthread_t _cs_tdiff0, _cs_tdiff1, _cs_tdiff2;
	pthread_create(&_cs_tdiff0, 0, t0, 0);
	pthread_create(&_cs_tdiff1, 0, t1, 0);
	pthread_create(&_cs_tdiff2, 0, t2, 0);
	return 0;
	}


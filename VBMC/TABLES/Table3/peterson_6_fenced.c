#include <pthread.h>


int level0, level1, level2, level3, level4, level5;
int waiting0, waiting1, waiting2, waiting3, waiting4;
int _cc_x;
int __fence_var;
void*t0(){
		while(1){
				level0 = 0;
		rmw(__fence_var, 0);
		waiting0=0;
		rmw(__fence_var, 0);
		assume(waiting0!=0 || (level1 < 0 && level2 < 0 && level3 < 0 && level4 < 0 && level5 < 0 && 1));
		level0 = 1;
		rmw(__fence_var, 0);
		waiting1=0;
		rmw(__fence_var, 0);
		assume(waiting1!=0 || (level1 < 1 && level2 < 1 && level3 < 1 && level4 < 1 && level5 < 1 && 1));
		level0 = 2;
		rmw(__fence_var, 0);
		waiting2=0;
		rmw(__fence_var, 0);
		assume(waiting2!=0 || (level1 < 2 && level2 < 2 && level3 < 2 && level4 < 2 && level5 < 2 && 1));
		level0 = 3;
		rmw(__fence_var, 0);
		waiting3=0;
		rmw(__fence_var, 0);
		assume(waiting3!=0 || (level1 < 3 && level2 < 3 && level3 < 3 && level4 < 3 && level5 < 3 && 1));
		level0 = 4;
		rmw(__fence_var, 0);
		waiting4=0;
		rmw(__fence_var, 0);
		assume(waiting4!=0 || (level1 < 4 && level2 < 4 && level3 < 4 && level4 < 4 && level5 < 4 && 1));
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
		assume(waiting0!=1 || (level0 < 0 && level2 < 0 && level3 < 0 && level4 < 0 && level5 < 0 && 1));
		level1 = 1;
		rmw(__fence_var, 0);
		waiting1=1;
		rmw(__fence_var, 0);
		assume(waiting1!=1 || (level0 < 1 && level2 < 1 && level3 < 1 && level4 < 1 && level5 < 1 && 1));
		level1 = 2;
		rmw(__fence_var, 0);
		waiting2=1;
		rmw(__fence_var, 0);
		assume(waiting2!=1 || (level0 < 2 && level2 < 2 && level3 < 2 && level4 < 2 && level5 < 2 && 1));
		level1 = 3;
		rmw(__fence_var, 0);
		waiting3=1;
		rmw(__fence_var, 0);
		assume(waiting3!=1 || (level0 < 3 && level2 < 3 && level3 < 3 && level4 < 3 && level5 < 3 && 1));
		level1 = 4;
		rmw(__fence_var, 0);
		waiting4=1;
		rmw(__fence_var, 0);
		assume(waiting4!=1 || (level0 < 4 && level2 < 4 && level3 < 4 && level4 < 4 && level5 < 4 && 1));
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
		assume(waiting0!=2 || (level0 < 0 && level1 < 0 && level3 < 0 && level4 < 0 && level5 < 0 && 1));
		level2 = 1;
		rmw(__fence_var, 0);
		waiting1=2;
		rmw(__fence_var, 0);
		assume(waiting1!=2 || (level0 < 1 && level1 < 1 && level3 < 1 && level4 < 1 && level5 < 1 && 1));
		level2 = 2;
		rmw(__fence_var, 0);
		waiting2=2;
		rmw(__fence_var, 0);
		assume(waiting2!=2 || (level0 < 2 && level1 < 2 && level3 < 2 && level4 < 2 && level5 < 2 && 1));
		level2 = 3;
		rmw(__fence_var, 0);
		waiting3=2;
		rmw(__fence_var, 0);
		assume(waiting3!=2 || (level0 < 3 && level1 < 3 && level3 < 3 && level4 < 3 && level5 < 3 && 1));
		level2 = 4;
		rmw(__fence_var, 0);
		waiting4=2;
		rmw(__fence_var, 0);
		assume(waiting4!=2 || (level0 < 4 && level1 < 4 && level3 < 4 && level4 < 4 && level5 < 4 && 1));
		_cc_x = 1;
		assert(_cc_x==2);
		rmw(__fence_var, 0);
		level2 = 0;
		}
}
void*t3(){
		while(1){
				level3 = 0;
		rmw(__fence_var, 0);
		waiting0=3;
		rmw(__fence_var, 0);
		assume(waiting0!=3 || (level0 < 0 && level1 < 0 && level2 < 0 && level4 < 0 && level5 < 0 && 1));
		level3 = 1;
		rmw(__fence_var, 0);
		waiting1=3;
		rmw(__fence_var, 0);
		assume(waiting1!=3 || (level0 < 1 && level1 < 1 && level2 < 1 && level4 < 1 && level5 < 1 && 1));
		level3 = 2;
		rmw(__fence_var, 0);
		waiting2=3;
		rmw(__fence_var, 0);
		assume(waiting2!=3 || (level0 < 2 && level1 < 2 && level2 < 2 && level4 < 2 && level5 < 2 && 1));
		level3 = 3;
		rmw(__fence_var, 0);
		waiting3=3;
		rmw(__fence_var, 0);
		assume(waiting3!=3 || (level0 < 3 && level1 < 3 && level2 < 3 && level4 < 3 && level5 < 3 && 1));
		level3 = 4;
		rmw(__fence_var, 0);
		waiting4=3;
		rmw(__fence_var, 0);
		assume(waiting4!=3 || (level0 < 4 && level1 < 4 && level2 < 4 && level4 < 4 && level5 < 4 && 1));
		_cc_x = 3;
		assert(_cc_x==3);
		rmw(__fence_var, 0);
		level3 = 0;
		}
}
void*t4(){
		while(1){
				level4 = 0;
		rmw(__fence_var, 0);
		waiting0=4;
		rmw(__fence_var, 0);
		assume(waiting0!=4 || (level0 < 0 && level1 < 0 && level2 < 0 && level3 < 0 && level5 < 0 && 1));
		level4 = 1;
		rmw(__fence_var, 0);
		waiting1=4;
		rmw(__fence_var, 0);
		assume(waiting1!=4 || (level0 < 1 && level1 < 1 && level2 < 1 && level3 < 1 && level5 < 1 && 1));
		level4 = 2;
		rmw(__fence_var, 0);
		waiting2=4;
		rmw(__fence_var, 0);
		assume(waiting2!=4 || (level0 < 2 && level1 < 2 && level2 < 2 && level3 < 2 && level5 < 2 && 1));
		level4 = 3;
		rmw(__fence_var, 0);
		waiting3=4;
		rmw(__fence_var, 0);
		assume(waiting3!=4 || (level0 < 3 && level1 < 3 && level2 < 3 && level3 < 3 && level5 < 3 && 1));
		level4 = 4;
		rmw(__fence_var, 0);
		waiting4=4;
		rmw(__fence_var, 0);
		assume(waiting4!=4 || (level0 < 4 && level1 < 4 && level2 < 4 && level3 < 4 && level5 < 4 && 1));
		_cc_x = 4;
		assert(_cc_x==4);
		rmw(__fence_var, 0);
		level4 = 0;
		}
}
void*t5(){
		while(1){
				level5 = 0;
		rmw(__fence_var, 0);
		waiting0=5;
		rmw(__fence_var, 0);
		assume(waiting0!=5 || (level0 < 0 && level1 < 0 && level2 < 0 && level3 < 0 && level4 < 0 && 1));
		level5 = 1;
		rmw(__fence_var, 0);
		waiting1=5;
		rmw(__fence_var, 0);
		assume(waiting1!=5 || (level0 < 1 && level1 < 1 && level2 < 1 && level3 < 1 && level4 < 1 && 1));
		level5 = 2;
		rmw(__fence_var, 0);
		waiting2=5;
		rmw(__fence_var, 0);
		assume(waiting2!=5 || (level0 < 2 && level1 < 2 && level2 < 2 && level3 < 2 && level4 < 2 && 1));
		level5 = 3;
		rmw(__fence_var, 0);
		waiting3=5;
		rmw(__fence_var, 0);
		assume(waiting3!=5 || (level0 < 3 && level1 < 3 && level2 < 3 && level3 < 3 && level4 < 3 && 1));
		level5 = 4;
		rmw(__fence_var, 0);
		waiting4=5;
		rmw(__fence_var, 0);
		assume(waiting4!=5 || (level0 < 4 && level1 < 4 && level2 < 4 && level3 < 4 && level4 < 4 && 1));
		_cc_x = 5;
		assert(_cc_x==5);
		rmw(__fence_var, 0);
		level5 = 0;
		}
}
int main(){
	pthread_t _cs_tdiff0, _cs_tdiff1, _cs_tdiff2, _cs_tdiff3, _cs_tdiff4, _cs_tdiff5;
	pthread_create(&_cs_tdiff0, 0, t0, 0);
	pthread_create(&_cs_tdiff1, 0, t1, 0);
	pthread_create(&_cs_tdiff2, 0, t2, 0);
	pthread_create(&_cs_tdiff3, 0, t3, 0);
	pthread_create(&_cs_tdiff4, 0, t4, 0);
	pthread_create(&_cs_tdiff5, 0, t5, 0);
	return 0;
	}


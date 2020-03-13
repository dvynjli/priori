#include <pthread.h>


int flag0, flag1, flag2, flag3, flag4;
int __fence_var;
int _cc_x;
void* t0(){
	while(1){
			flag0=1;
		rmw(__fence_var, 0);
		assume(flag1<3 && flag2<3 && flag3<3 && flag4<3 && 1);
		flag0=3;
		rmw(__fence_var, 0);
		if(flag1==1 || flag2==1 || flag3==1 || flag4==1){
			flag0 = 2;
			rmw(__fence_var, 0);
			assume(flag1==4 || flag2==4 || flag3==4 || flag4==4);
			}
			flag0 = 4;
		rmw(__fence_var, 0);
		_cc_x = 0;
		assert(_cc_x==0);
		assume(flag1 != 3 && flag1 != 2 && flag2 != 3 && flag2 != 2 && flag3 != 3 && flag3 != 2 && flag4 != 3 && flag4 != 2 );
		flag0 = 0;
		}
		}
void* t1(){
	while(1){
			flag1=1;
		rmw(__fence_var, 0);
		assume(flag0<3 && flag2<3 && flag3<3 && flag4<3 && 1);
		flag1=3;
		rmw(__fence_var, 0);
		if(flag0==1 || flag2==1 || flag3==1 || flag4==1){
			flag1 = 2;
			rmw(__fence_var, 0);
			assume(flag0==4 || flag2==4 || flag3==4 || flag4==4);
			}
			flag1 = 4;
		rmw(__fence_var, 0);
		assume(flag0 < 2);
		_cc_x = 1;
		assert(_cc_x==1);
		assume(flag2 != 3 && flag2 != 2 && flag3 != 3 && flag3 != 2 && flag4 != 3 && flag4 != 2 );
		flag1 = 0;
		}
		}
void* t2(){
	while(1){
			flag2=1;
		rmw(__fence_var, 0);
		assume(flag0<3 && flag1<3 && flag3<3 && flag4<3 && 1);
		flag2=3;
		rmw(__fence_var, 0);
		if(flag0==1 || flag1==1 || flag3==1 || flag4==1){
			flag2 = 2;
			rmw(__fence_var, 0);
			assume(flag0==4 || flag1==4 || flag3==4 || flag4==4);
			}
			flag2 = 4;
		rmw(__fence_var, 0);
		assume(flag0 < 2 && flag1 < 2);
		_cc_x = 1;
		assert(_cc_x==2);
		assume(flag3 != 3 && flag3 != 2 && flag4 != 3 && flag4 != 2 );
		flag2 = 0;
		}
		}
void* t3(){
	while(1){
			flag3=1;
		rmw(__fence_var, 0);
		assume(flag0<3 && flag1<3 && flag2<3 && flag4<3 && 1);
		flag3=3;
		rmw(__fence_var, 0);
		if(flag0==1 || flag1==1 || flag2==1 || flag4==1){
			flag3 = 2;
			rmw(__fence_var, 0);
			assume(flag0==4 || flag1==4 || flag2==4 || flag4==4);
			}
			flag3 = 4;
		rmw(__fence_var, 0);
		assume(flag0 < 2 && flag1 < 2 && flag2 < 2);
		_cc_x = 3;
		assert(_cc_x==3);
		assume(flag4 != 3 && flag4 != 2 );
		flag3 = 0;
		}
		}
void* t4(){
	while(1){
			flag4=1;
		rmw(__fence_var, 0);
		assume(flag0<3 && flag1<3 && flag2<3 && flag3<3 && 1);
		flag4=3;
		rmw(__fence_var, 0);
		if(flag0==1 || flag1==1 || flag2==1 || flag3==1){
			flag4 = 2;
			rmw(__fence_var, 0);
			assume(flag0==4 || flag1==4 || flag2==4 || flag3==4);
			}
			flag4 = 4;
		rmw(__fence_var, 0);
		assume(flag0 < 2 && flag1 < 2 && flag2 < 2 && flag3 < 2);
		_cc_x = 4;
		assert(_cc_x==4);
		flag4 = 0;
		}
		}
int main(){
	pthread_t _cs_tdiff0, _cs_tdiff1, _cs_tdiff2, _cs_tdiff3, _cs_tdiff4;
	pthread_create(&_cs_tdiff0, 0, t0, 0);
	pthread_create(&_cs_tdiff1, 0, t1, 0);
	pthread_create(&_cs_tdiff2, 0, t2, 0);
	pthread_create(&_cs_tdiff3, 0, t3, 0);
	pthread_create(&_cs_tdiff4, 0, t4, 0);
	return 0;
	}


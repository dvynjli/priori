#include <pthread.h>


int flag0, flag1, flag2, flag3, flag4, flag5, flag6, flag7;
int __fence_var;
int _cc_x;
void* t0(){
	while(1){
			flag0=1;
		rmw(__fence_var, 0);
		assume(flag1<3 && flag2<3 && flag3<3 && flag4<3 && flag5<3 && flag6<3 && flag7<3 && 1);
		flag0=3;
		rmw(__fence_var, 0);
		if(flag1==1 || flag2==1 || flag3==1 || flag4==1 || flag5==1 || flag6==1 || flag7==1){
			flag0 = 2;
			rmw(__fence_var, 0);
			assume(flag1==4 || flag2==4 || flag3==4 || flag4==4 || flag5==4 || flag6==4 || flag7==4);
			}
			flag0 = 4;
		rmw(__fence_var, 0);
		_cc_x = 0;
		assert(_cc_x==0);
		assume(flag1 != 3 && flag1 != 2 && flag2 != 3 && flag2 != 2 && flag3 != 3 && flag3 != 2 && flag4 != 3 && flag4 != 2 && flag5 != 3 && flag5 != 2 && flag6 != 3 && flag6 != 2 && flag7 != 3 && flag7 != 2 );
		flag0 = 0;
		}
		}
void* t1(){
	while(1){
			flag1=1;
		rmw(__fence_var, 0);
		assume(flag0<3 && flag2<3 && flag3<3 && flag4<3 && flag5<3 && flag6<3 && flag7<3 && 1);
		flag1=3;
		rmw(__fence_var, 0);
		if(flag0==1 || flag2==1 || flag3==1 || flag4==1 || flag5==1 || flag6==1 || flag7==1){
			flag1 = 2;
			rmw(__fence_var, 0);
			assume(flag0==4 || flag2==4 || flag3==4 || flag4==4 || flag5==4 || flag6==4 || flag7==4);
			}
			flag1 = 4;
		rmw(__fence_var, 0);
		assume(flag0 < 2);
		_cc_x = 1;
		assert(_cc_x==1);
		assume(flag2 != 3 && flag2 != 2 && flag3 != 3 && flag3 != 2 && flag4 != 3 && flag4 != 2 && flag5 != 3 && flag5 != 2 && flag6 != 3 && flag6 != 2 && flag7 != 3 && flag7 != 2 );
		flag1 = 0;
		}
		}
void* t2(){
	while(1){
			flag2=1;
		
		assume(flag0<3 && flag1<3 && flag3<3 && flag4<3 && flag5<3 && flag6<3 && flag7<3 && 1);
		flag2=3;
		
		if(flag0==1 || flag1==1 || flag3==1 || flag4==1 || flag5==1 || flag6==1 || flag7==1){
			flag2 = 2;
			
			assume(flag0==4 || flag1==4 || flag3==4 || flag4==4 || flag5==4 || flag6==4 || flag7==4);
			}
			flag2 = 4;
		
		assume(flag0 < 2 && flag1 < 2);
		_cc_x = 2;
		assert(_cc_x==2);
		assume(flag3 != 3 && flag3 != 2 && flag4 != 3 && flag4 != 2 && flag5 != 3 && flag5 != 2 && flag6 != 3 && flag6 != 2 && flag7 != 3 && flag7 != 2 );
		flag2 = 0;
		}
		}
void* t3(){
	while(1){
			flag3=1;
		rmw(__fence_var, 0);
		assume(flag0<3 && flag1<3 && flag2<3 && flag4<3 && flag5<3 && flag6<3 && flag7<3 && 1);
		flag3=3;
		rmw(__fence_var, 0);
		if(flag0==1 || flag1==1 || flag2==1 || flag4==1 || flag5==1 || flag6==1 || flag7==1){
			flag3 = 2;
			rmw(__fence_var, 0);
			assume(flag0==4 || flag1==4 || flag2==4 || flag4==4 || flag5==4 || flag6==4 || flag7==4);
			}
			flag3 = 4;
		rmw(__fence_var, 0);
		assume(flag0 < 2 && flag1 < 2 && flag2 < 2);
		_cc_x = 3;
		assert(_cc_x==3);
		assume(flag4 != 3 && flag4 != 2 && flag5 != 3 && flag5 != 2 && flag6 != 3 && flag6 != 2 && flag7 != 3 && flag7 != 2 );
		flag3 = 0;
		}
		}
void* t4(){
	while(1){
			flag4=1;
		rmw(__fence_var, 0);
		assume(flag0<3 && flag1<3 && flag2<3 && flag3<3 && flag5<3 && flag6<3 && flag7<3 && 1);
		flag4=3;
		rmw(__fence_var, 0);
		if(flag0==1 || flag1==1 || flag2==1 || flag3==1 || flag5==1 || flag6==1 || flag7==1){
			flag4 = 2;
			rmw(__fence_var, 0);
			assume(flag0==4 || flag1==4 || flag2==4 || flag3==4 || flag5==4 || flag6==4 || flag7==4);
			}
			flag4 = 4;
		rmw(__fence_var, 0);
		assume(flag0 < 2 && flag1 < 2 && flag2 < 2 && flag3 < 2);
		_cc_x = 4;
		assert(_cc_x==4);
		assume(flag5 != 3 && flag5 != 2 && flag6 != 3 && flag6 != 2 && flag7 != 3 && flag7 != 2 );
		flag4 = 0;
		}
		}
void* t5(){
	while(1){
			flag5=1;
		rmw(__fence_var, 0);
		assume(flag0<3 && flag1<3 && flag2<3 && flag3<3 && flag4<3 && flag6<3 && flag7<3 && 1);
		flag5=3;
		rmw(__fence_var, 0);
		if(flag0==1 || flag1==1 || flag2==1 || flag3==1 || flag4==1 || flag6==1 || flag7==1){
			flag5 = 2;
			rmw(__fence_var, 0);
			assume(flag0==4 || flag1==4 || flag2==4 || flag3==4 || flag4==4 || flag6==4 || flag7==4);
			}
			flag5 = 4;
		rmw(__fence_var, 0);
		assume(flag0 < 2 && flag1 < 2 && flag2 < 2 && flag3 < 2 && flag4 < 2);
		_cc_x = 5;
		assert(_cc_x==5);
		assume(flag6 != 3 && flag6 != 2 && flag7 != 3 && flag7 != 2 );
		flag5 = 0;
		}
		}
void* t6(){
	while(1){
			flag6=1;
		rmw(__fence_var, 0);
		assume(flag0<3 && flag1<3 && flag2<3 && flag3<3 && flag4<3 && flag5<3 && flag7<3 && 1);
		flag6=3;
		rmw(__fence_var, 0);
		if(flag0==1 || flag1==1 || flag2==1 || flag3==1 || flag4==1 || flag5==1 || flag7==1){
			flag6 = 2;
			rmw(__fence_var, 0);
			assume(flag0==4 || flag1==4 || flag2==4 || flag3==4 || flag4==4 || flag5==4 || flag7==4);
			}
			flag6 = 4;
		rmw(__fence_var, 0);
		assume(flag0 < 2 && flag1 < 2 && flag2 < 2 && flag3 < 2 && flag4 < 2 && flag5 < 2);
		_cc_x = 6;
		assert(_cc_x==6);
		assume(flag7 != 3 && flag7 != 2 );
		flag6 = 0;
		}
		}
void* t7(){
	while(1){
			flag7=1;
		rmw(__fence_var, 0);
		assume(flag0<3 && flag1<3 && flag2<3 && flag3<3 && flag4<3 && flag5<3 && flag6<3 && 1);
		flag7=3;
		rmw(__fence_var, 0);
		if(flag0==1 || flag1==1 || flag2==1 || flag3==1 || flag4==1 || flag5==1 || flag6==1){
			flag7 = 2;
			rmw(__fence_var, 0);
			assume(flag0==4 || flag1==4 || flag2==4 || flag3==4 || flag4==4 || flag5==4 || flag6==4);
			}
			flag7 = 4;
		rmw(__fence_var, 0);
		assume(flag0 < 2 && flag1 < 2 && flag2 < 2 && flag3 < 2 && flag4 < 2 && flag5 < 2 && flag6 < 2);
		_cc_x = 7;
		assert(_cc_x==7);
		flag7 = 0;
		}
		}
int main(){
	pthread_t _cs_tdiff0, _cs_tdiff1, _cs_tdiff2, _cs_tdiff3, _cs_tdiff4, _cs_tdiff5, _cs_tdiff6, _cs_tdiff7;
	pthread_create(&_cs_tdiff0, 0, t0, 0);
	pthread_create(&_cs_tdiff1, 0, t1, 0);
	pthread_create(&_cs_tdiff2, 0, t2, 0);
	pthread_create(&_cs_tdiff3, 0, t3, 0);
	pthread_create(&_cs_tdiff4, 0, t4, 0);
	pthread_create(&_cs_tdiff5, 0, t5, 0);
	pthread_create(&_cs_tdiff6, 0, t6, 0);
	pthread_create(&_cs_tdiff7, 0, t7, 0);
	return 0;
	}


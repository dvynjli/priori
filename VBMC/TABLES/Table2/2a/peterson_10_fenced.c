#include <pthread.h>


int level0, level1, level2, level3, level4, level5, level6, level7, level8, level9;
int waiting0, waiting1, waiting2, waiting3, waiting4, waiting5, waiting6, waiting7, waiting8;
int _cc_x;
int __fence_var;
void*t0(){
		while(1){
				level0 = 0;
		rmw(__fence_var, 0);
		waiting0=0;
		rmw(__fence_var, 0);
		assume(waiting0!=0 || (level1 < 0 && level2 < 0 && level3 < 0 && level4 < 0 && level5 < 0 && level6 < 0 && level7 < 0 && level8 < 0 && level9 < 0 && 1));
		level0 = 1;
		rmw(__fence_var, 0);
		waiting1=0;
		rmw(__fence_var, 0);
		assume(waiting1!=0 || (level1 < 1 && level2 < 1 && level3 < 1 && level4 < 1 && level5 < 1 && level6 < 1 && level7 < 1 && level8 < 1 && level9 < 1 && 1));
		level0 = 2;
		rmw(__fence_var, 0);
		waiting2=0;
		rmw(__fence_var, 0);
		assume(waiting2!=0 || (level1 < 2 && level2 < 2 && level3 < 2 && level4 < 2 && level5 < 2 && level6 < 2 && level7 < 2 && level8 < 2 && level9 < 2 && 1));
		level0 = 3;
		rmw(__fence_var, 0);
		waiting3=0;
		rmw(__fence_var, 0);
		assume(waiting3!=0 || (level1 < 3 && level2 < 3 && level3 < 3 && level4 < 3 && level5 < 3 && level6 < 3 && level7 < 3 && level8 < 3 && level9 < 3 && 1));
		level0 = 4;
		rmw(__fence_var, 0);
		waiting4=0;
		rmw(__fence_var, 0);
		assume(waiting4!=0 || (level1 < 4 && level2 < 4 && level3 < 4 && level4 < 4 && level5 < 4 && level6 < 4 && level7 < 4 && level8 < 4 && level9 < 4 && 1));
		level0 = 5;
		rmw(__fence_var, 0);
		waiting5=0;
		rmw(__fence_var, 0);
		assume(waiting5!=0 || (level1 < 5 && level2 < 5 && level3 < 5 && level4 < 5 && level5 < 5 && level6 < 5 && level7 < 5 && level8 < 5 && level9 < 5 && 1));
		level0 = 6;
		rmw(__fence_var, 0);
		waiting6=0;
		rmw(__fence_var, 0);
		assume(waiting6!=0 || (level1 < 6 && level2 < 6 && level3 < 6 && level4 < 6 && level5 < 6 && level6 < 6 && level7 < 6 && level8 < 6 && level9 < 6 && 1));
		level0 = 7;
		rmw(__fence_var, 0);
		waiting7=0;
		rmw(__fence_var, 0);
		assume(waiting7!=0 || (level1 < 7 && level2 < 7 && level3 < 7 && level4 < 7 && level5 < 7 && level6 < 7 && level7 < 7 && level8 < 7 && level9 < 7 && 1));
		level0 = 8;
		rmw(__fence_var, 0);
		waiting8=0;
		rmw(__fence_var, 0);
		assume(waiting8!=0 || (level1 < 8 && level2 < 8 && level3 < 8 && level4 < 8 && level5 < 8 && level6 < 8 && level7 < 8 && level8 < 8 && level9 < 8 && 1));
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
		assume(waiting0!=1 || (level0 < 0 && level2 < 0 && level3 < 0 && level4 < 0 && level5 < 0 && level6 < 0 && level7 < 0 && level8 < 0 && level9 < 0 && 1));
		level1 = 1;
		rmw(__fence_var, 0);
		waiting1=1;
		rmw(__fence_var, 0);
		assume(waiting1!=1 || (level0 < 1 && level2 < 1 && level3 < 1 && level4 < 1 && level5 < 1 && level6 < 1 && level7 < 1 && level8 < 1 && level9 < 1 && 1));
		level1 = 2;
		rmw(__fence_var, 0);
		waiting2=1;
		rmw(__fence_var, 0);
		assume(waiting2!=1 || (level0 < 2 && level2 < 2 && level3 < 2 && level4 < 2 && level5 < 2 && level6 < 2 && level7 < 2 && level8 < 2 && level9 < 2 && 1));
		level1 = 3;
		rmw(__fence_var, 0);
		waiting3=1;
		rmw(__fence_var, 0);
		assume(waiting3!=1 || (level0 < 3 && level2 < 3 && level3 < 3 && level4 < 3 && level5 < 3 && level6 < 3 && level7 < 3 && level8 < 3 && level9 < 3 && 1));
		level1 = 4;
		rmw(__fence_var, 0);
		waiting4=1;
		rmw(__fence_var, 0);
		assume(waiting4!=1 || (level0 < 4 && level2 < 4 && level3 < 4 && level4 < 4 && level5 < 4 && level6 < 4 && level7 < 4 && level8 < 4 && level9 < 4 && 1));
		level1 = 5;
		rmw(__fence_var, 0);
		waiting5=1;
		rmw(__fence_var, 0);
		assume(waiting5!=1 || (level0 < 5 && level2 < 5 && level3 < 5 && level4 < 5 && level5 < 5 && level6 < 5 && level7 < 5 && level8 < 5 && level9 < 5 && 1));
		level1 = 6;
		rmw(__fence_var, 0);
		waiting6=1;
		rmw(__fence_var, 0);
		assume(waiting6!=1 || (level0 < 6 && level2 < 6 && level3 < 6 && level4 < 6 && level5 < 6 && level6 < 6 && level7 < 6 && level8 < 6 && level9 < 6 && 1));
		level1 = 7;
		rmw(__fence_var, 0);
		waiting7=1;
		rmw(__fence_var, 0);
		assume(waiting7!=1 || (level0 < 7 && level2 < 7 && level3 < 7 && level4 < 7 && level5 < 7 && level6 < 7 && level7 < 7 && level8 < 7 && level9 < 7 && 1));
		level1 = 8;
		rmw(__fence_var, 0);
		waiting8=1;
		rmw(__fence_var, 0);
		assume(waiting8!=1 || (level0 < 8 && level2 < 8 && level3 < 8 && level4 < 8 && level5 < 8 && level6 < 8 && level7 < 8 && level8 < 8 && level9 < 8 && 1));
		_cc_x = 1;
		assert(_cc_x==1);
		rmw(__fence_var, 0);
		level1 = 0;
		}
}
void*t2(){
		while(1){
				level2 = 0;
		
		waiting0=2;
		
		assume(waiting0!=2 || (level0 < 0 && level1 < 0 && level3 < 0 && level4 < 0 && level5 < 0 && level6 < 0 && level7 < 0 && level8 < 0 && level9 < 0 && 1));
		level2 = 1;
		
		waiting1=2;
		
		assume(waiting1!=2 || (level0 < 1 && level1 < 1 && level3 < 1 && level4 < 1 && level5 < 1 && level6 < 1 && level7 < 1 && level8 < 1 && level9 < 1 && 1));
		level2 = 2;
		
		waiting2=2;
		
		assume(waiting2!=2 || (level0 < 2 && level1 < 2 && level3 < 2 && level4 < 2 && level5 < 2 && level6 < 2 && level7 < 2 && level8 < 2 && level9 < 2 && 1));
		level2 = 3;
		
		waiting3=2;
		
		assume(waiting3!=2 || (level0 < 3 && level1 < 3 && level3 < 3 && level4 < 3 && level5 < 3 && level6 < 3 && level7 < 3 && level8 < 3 && level9 < 3 && 1));
		level2 = 4;
		
		waiting4=2;
		
		assume(waiting4!=2 || (level0 < 4 && level1 < 4 && level3 < 4 && level4 < 4 && level5 < 4 && level6 < 4 && level7 < 4 && level8 < 4 && level9 < 4 && 1));
		level2 = 5;
		
		waiting5=2;
		
		assume(waiting5!=2 || (level0 < 5 && level1 < 5 && level3 < 5 && level4 < 5 && level5 < 5 && level6 < 5 && level7 < 5 && level8 < 5 && level9 < 5 && 1));
		level2 = 6;
		
		waiting6=2;
		
		assume(waiting6!=2 || (level0 < 6 && level1 < 6 && level3 < 6 && level4 < 6 && level5 < 6 && level6 < 6 && level7 < 6 && level8 < 6 && level9 < 6 && 1));
		level2 = 7;
		
		waiting7=2;
		
		assume(waiting7!=2 || (level0 < 7 && level1 < 7 && level3 < 7 && level4 < 7 && level5 < 7 && level6 < 7 && level7 < 7 && level8 < 7 && level9 < 7 && 1));
		level2 = 8;
		
		waiting8=2;
		
		assume(waiting8!=2 || (level0 < 8 && level1 < 8 && level3 < 8 && level4 < 8 && level5 < 8 && level6 < 8 && level7 < 8 && level8 < 8 && level9 < 8 && 1));
		_cc_x = 2;
		assert(_cc_x==2);
		
		level2 = 0;
		}
}
void*t3(){
		while(1){
				level3 = 0;
		rmw(__fence_var, 0);
		waiting0=3;
		rmw(__fence_var, 0);
		assume(waiting0!=3 || (level0 < 0 && level1 < 0 && level2 < 0 && level4 < 0 && level5 < 0 && level6 < 0 && level7 < 0 && level8 < 0 && level9 < 0 && 1));
		level3 = 1;
		rmw(__fence_var, 0);
		waiting1=3;
		rmw(__fence_var, 0);
		assume(waiting1!=3 || (level0 < 1 && level1 < 1 && level2 < 1 && level4 < 1 && level5 < 1 && level6 < 1 && level7 < 1 && level8 < 1 && level9 < 1 && 1));
		level3 = 2;
		rmw(__fence_var, 0);
		waiting2=3;
		rmw(__fence_var, 0);
		assume(waiting2!=3 || (level0 < 2 && level1 < 2 && level2 < 2 && level4 < 2 && level5 < 2 && level6 < 2 && level7 < 2 && level8 < 2 && level9 < 2 && 1));
		level3 = 3;
		rmw(__fence_var, 0);
		waiting3=3;
		rmw(__fence_var, 0);
		assume(waiting3!=3 || (level0 < 3 && level1 < 3 && level2 < 3 && level4 < 3 && level5 < 3 && level6 < 3 && level7 < 3 && level8 < 3 && level9 < 3 && 1));
		level3 = 4;
		rmw(__fence_var, 0);
		waiting4=3;
		rmw(__fence_var, 0);
		assume(waiting4!=3 || (level0 < 4 && level1 < 4 && level2 < 4 && level4 < 4 && level5 < 4 && level6 < 4 && level7 < 4 && level8 < 4 && level9 < 4 && 1));
		level3 = 5;
		rmw(__fence_var, 0);
		waiting5=3;
		rmw(__fence_var, 0);
		assume(waiting5!=3 || (level0 < 5 && level1 < 5 && level2 < 5 && level4 < 5 && level5 < 5 && level6 < 5 && level7 < 5 && level8 < 5 && level9 < 5 && 1));
		level3 = 6;
		rmw(__fence_var, 0);
		waiting6=3;
		rmw(__fence_var, 0);
		assume(waiting6!=3 || (level0 < 6 && level1 < 6 && level2 < 6 && level4 < 6 && level5 < 6 && level6 < 6 && level7 < 6 && level8 < 6 && level9 < 6 && 1));
		level3 = 7;
		rmw(__fence_var, 0);
		waiting7=3;
		rmw(__fence_var, 0);
		assume(waiting7!=3 || (level0 < 7 && level1 < 7 && level2 < 7 && level4 < 7 && level5 < 7 && level6 < 7 && level7 < 7 && level8 < 7 && level9 < 7 && 1));
		level3 = 8;
		rmw(__fence_var, 0);
		waiting8=3;
		rmw(__fence_var, 0);
		assume(waiting8!=3 || (level0 < 8 && level1 < 8 && level2 < 8 && level4 < 8 && level5 < 8 && level6 < 8 && level7 < 8 && level8 < 8 && level9 < 8 && 1));
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
		assume(waiting0!=4 || (level0 < 0 && level1 < 0 && level2 < 0 && level3 < 0 && level5 < 0 && level6 < 0 && level7 < 0 && level8 < 0 && level9 < 0 && 1));
		level4 = 1;
		rmw(__fence_var, 0);
		waiting1=4;
		rmw(__fence_var, 0);
		assume(waiting1!=4 || (level0 < 1 && level1 < 1 && level2 < 1 && level3 < 1 && level5 < 1 && level6 < 1 && level7 < 1 && level8 < 1 && level9 < 1 && 1));
		level4 = 2;
		rmw(__fence_var, 0);
		waiting2=4;
		rmw(__fence_var, 0);
		assume(waiting2!=4 || (level0 < 2 && level1 < 2 && level2 < 2 && level3 < 2 && level5 < 2 && level6 < 2 && level7 < 2 && level8 < 2 && level9 < 2 && 1));
		level4 = 3;
		rmw(__fence_var, 0);
		waiting3=4;
		rmw(__fence_var, 0);
		assume(waiting3!=4 || (level0 < 3 && level1 < 3 && level2 < 3 && level3 < 3 && level5 < 3 && level6 < 3 && level7 < 3 && level8 < 3 && level9 < 3 && 1));
		level4 = 4;
		rmw(__fence_var, 0);
		waiting4=4;
		rmw(__fence_var, 0);
		assume(waiting4!=4 || (level0 < 4 && level1 < 4 && level2 < 4 && level3 < 4 && level5 < 4 && level6 < 4 && level7 < 4 && level8 < 4 && level9 < 4 && 1));
		level4 = 5;
		rmw(__fence_var, 0);
		waiting5=4;
		rmw(__fence_var, 0);
		assume(waiting5!=4 || (level0 < 5 && level1 < 5 && level2 < 5 && level3 < 5 && level5 < 5 && level6 < 5 && level7 < 5 && level8 < 5 && level9 < 5 && 1));
		level4 = 6;
		rmw(__fence_var, 0);
		waiting6=4;
		rmw(__fence_var, 0);
		assume(waiting6!=4 || (level0 < 6 && level1 < 6 && level2 < 6 && level3 < 6 && level5 < 6 && level6 < 6 && level7 < 6 && level8 < 6 && level9 < 6 && 1));
		level4 = 7;
		rmw(__fence_var, 0);
		waiting7=4;
		rmw(__fence_var, 0);
		assume(waiting7!=4 || (level0 < 7 && level1 < 7 && level2 < 7 && level3 < 7 && level5 < 7 && level6 < 7 && level7 < 7 && level8 < 7 && level9 < 7 && 1));
		level4 = 8;
		rmw(__fence_var, 0);
		waiting8=4;
		rmw(__fence_var, 0);
		assume(waiting8!=4 || (level0 < 8 && level1 < 8 && level2 < 8 && level3 < 8 && level5 < 8 && level6 < 8 && level7 < 8 && level8 < 8 && level9 < 8 && 1));
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
		assume(waiting0!=5 || (level0 < 0 && level1 < 0 && level2 < 0 && level3 < 0 && level4 < 0 && level6 < 0 && level7 < 0 && level8 < 0 && level9 < 0 && 1));
		level5 = 1;
		rmw(__fence_var, 0);
		waiting1=5;
		rmw(__fence_var, 0);
		assume(waiting1!=5 || (level0 < 1 && level1 < 1 && level2 < 1 && level3 < 1 && level4 < 1 && level6 < 1 && level7 < 1 && level8 < 1 && level9 < 1 && 1));
		level5 = 2;
		rmw(__fence_var, 0);
		waiting2=5;
		rmw(__fence_var, 0);
		assume(waiting2!=5 || (level0 < 2 && level1 < 2 && level2 < 2 && level3 < 2 && level4 < 2 && level6 < 2 && level7 < 2 && level8 < 2 && level9 < 2 && 1));
		level5 = 3;
		rmw(__fence_var, 0);
		waiting3=5;
		rmw(__fence_var, 0);
		assume(waiting3!=5 || (level0 < 3 && level1 < 3 && level2 < 3 && level3 < 3 && level4 < 3 && level6 < 3 && level7 < 3 && level8 < 3 && level9 < 3 && 1));
		level5 = 4;
		rmw(__fence_var, 0);
		waiting4=5;
		rmw(__fence_var, 0);
		assume(waiting4!=5 || (level0 < 4 && level1 < 4 && level2 < 4 && level3 < 4 && level4 < 4 && level6 < 4 && level7 < 4 && level8 < 4 && level9 < 4 && 1));
		level5 = 5;
		rmw(__fence_var, 0);
		waiting5=5;
		rmw(__fence_var, 0);
		assume(waiting5!=5 || (level0 < 5 && level1 < 5 && level2 < 5 && level3 < 5 && level4 < 5 && level6 < 5 && level7 < 5 && level8 < 5 && level9 < 5 && 1));
		level5 = 6;
		rmw(__fence_var, 0);
		waiting6=5;
		rmw(__fence_var, 0);
		assume(waiting6!=5 || (level0 < 6 && level1 < 6 && level2 < 6 && level3 < 6 && level4 < 6 && level6 < 6 && level7 < 6 && level8 < 6 && level9 < 6 && 1));
		level5 = 7;
		rmw(__fence_var, 0);
		waiting7=5;
		rmw(__fence_var, 0);
		assume(waiting7!=5 || (level0 < 7 && level1 < 7 && level2 < 7 && level3 < 7 && level4 < 7 && level6 < 7 && level7 < 7 && level8 < 7 && level9 < 7 && 1));
		level5 = 8;
		rmw(__fence_var, 0);
		waiting8=5;
		rmw(__fence_var, 0);
		assume(waiting8!=5 || (level0 < 8 && level1 < 8 && level2 < 8 && level3 < 8 && level4 < 8 && level6 < 8 && level7 < 8 && level8 < 8 && level9 < 8 && 1));
		_cc_x = 5;
		assert(_cc_x==5);
		rmw(__fence_var, 0);
		level5 = 0;
		}
}
void*t6(){
		while(1){
				level6 = 0;
		rmw(__fence_var, 0);
		waiting0=6;
		rmw(__fence_var, 0);
		assume(waiting0!=6 || (level0 < 0 && level1 < 0 && level2 < 0 && level3 < 0 && level4 < 0 && level5 < 0 && level7 < 0 && level8 < 0 && level9 < 0 && 1));
		level6 = 1;
		rmw(__fence_var, 0);
		waiting1=6;
		rmw(__fence_var, 0);
		assume(waiting1!=6 || (level0 < 1 && level1 < 1 && level2 < 1 && level3 < 1 && level4 < 1 && level5 < 1 && level7 < 1 && level8 < 1 && level9 < 1 && 1));
		level6 = 2;
		rmw(__fence_var, 0);
		waiting2=6;
		rmw(__fence_var, 0);
		assume(waiting2!=6 || (level0 < 2 && level1 < 2 && level2 < 2 && level3 < 2 && level4 < 2 && level5 < 2 && level7 < 2 && level8 < 2 && level9 < 2 && 1));
		level6 = 3;
		rmw(__fence_var, 0);
		waiting3=6;
		rmw(__fence_var, 0);
		assume(waiting3!=6 || (level0 < 3 && level1 < 3 && level2 < 3 && level3 < 3 && level4 < 3 && level5 < 3 && level7 < 3 && level8 < 3 && level9 < 3 && 1));
		level6 = 4;
		rmw(__fence_var, 0);
		waiting4=6;
		rmw(__fence_var, 0);
		assume(waiting4!=6 || (level0 < 4 && level1 < 4 && level2 < 4 && level3 < 4 && level4 < 4 && level5 < 4 && level7 < 4 && level8 < 4 && level9 < 4 && 1));
		level6 = 5;
		rmw(__fence_var, 0);
		waiting5=6;
		rmw(__fence_var, 0);
		assume(waiting5!=6 || (level0 < 5 && level1 < 5 && level2 < 5 && level3 < 5 && level4 < 5 && level5 < 5 && level7 < 5 && level8 < 5 && level9 < 5 && 1));
		level6 = 6;
		rmw(__fence_var, 0);
		waiting6=6;
		rmw(__fence_var, 0);
		assume(waiting6!=6 || (level0 < 6 && level1 < 6 && level2 < 6 && level3 < 6 && level4 < 6 && level5 < 6 && level7 < 6 && level8 < 6 && level9 < 6 && 1));
		level6 = 7;
		rmw(__fence_var, 0);
		waiting7=6;
		rmw(__fence_var, 0);
		assume(waiting7!=6 || (level0 < 7 && level1 < 7 && level2 < 7 && level3 < 7 && level4 < 7 && level5 < 7 && level7 < 7 && level8 < 7 && level9 < 7 && 1));
		level6 = 8;
		rmw(__fence_var, 0);
		waiting8=6;
		rmw(__fence_var, 0);
		assume(waiting8!=6 || (level0 < 8 && level1 < 8 && level2 < 8 && level3 < 8 && level4 < 8 && level5 < 8 && level7 < 8 && level8 < 8 && level9 < 8 && 1));
		_cc_x = 6;
		assert(_cc_x==6);
		rmw(__fence_var, 0);
		level6 = 0;
		}
}
void*t7(){
		while(1){
				level7 = 0;
		rmw(__fence_var, 0);
		waiting0=7;
		rmw(__fence_var, 0);
		assume(waiting0!=7 || (level0 < 0 && level1 < 0 && level2 < 0 && level3 < 0 && level4 < 0 && level5 < 0 && level6 < 0 && level8 < 0 && level9 < 0 && 1));
		level7 = 1;
		rmw(__fence_var, 0);
		waiting1=7;
		rmw(__fence_var, 0);
		assume(waiting1!=7 || (level0 < 1 && level1 < 1 && level2 < 1 && level3 < 1 && level4 < 1 && level5 < 1 && level6 < 1 && level8 < 1 && level9 < 1 && 1));
		level7 = 2;
		rmw(__fence_var, 0);
		waiting2=7;
		rmw(__fence_var, 0);
		assume(waiting2!=7 || (level0 < 2 && level1 < 2 && level2 < 2 && level3 < 2 && level4 < 2 && level5 < 2 && level6 < 2 && level8 < 2 && level9 < 2 && 1));
		level7 = 3;
		rmw(__fence_var, 0);
		waiting3=7;
		rmw(__fence_var, 0);
		assume(waiting3!=7 || (level0 < 3 && level1 < 3 && level2 < 3 && level3 < 3 && level4 < 3 && level5 < 3 && level6 < 3 && level8 < 3 && level9 < 3 && 1));
		level7 = 4;
		rmw(__fence_var, 0);
		waiting4=7;
		rmw(__fence_var, 0);
		assume(waiting4!=7 || (level0 < 4 && level1 < 4 && level2 < 4 && level3 < 4 && level4 < 4 && level5 < 4 && level6 < 4 && level8 < 4 && level9 < 4 && 1));
		level7 = 5;
		rmw(__fence_var, 0);
		waiting5=7;
		rmw(__fence_var, 0);
		assume(waiting5!=7 || (level0 < 5 && level1 < 5 && level2 < 5 && level3 < 5 && level4 < 5 && level5 < 5 && level6 < 5 && level8 < 5 && level9 < 5 && 1));
		level7 = 6;
		rmw(__fence_var, 0);
		waiting6=7;
		rmw(__fence_var, 0);
		assume(waiting6!=7 || (level0 < 6 && level1 < 6 && level2 < 6 && level3 < 6 && level4 < 6 && level5 < 6 && level6 < 6 && level8 < 6 && level9 < 6 && 1));
		level7 = 7;
		rmw(__fence_var, 0);
		waiting7=7;
		rmw(__fence_var, 0);
		assume(waiting7!=7 || (level0 < 7 && level1 < 7 && level2 < 7 && level3 < 7 && level4 < 7 && level5 < 7 && level6 < 7 && level8 < 7 && level9 < 7 && 1));
		level7 = 8;
		rmw(__fence_var, 0);
		waiting8=7;
		rmw(__fence_var, 0);
		assume(waiting8!=7 || (level0 < 8 && level1 < 8 && level2 < 8 && level3 < 8 && level4 < 8 && level5 < 8 && level6 < 8 && level8 < 8 && level9 < 8 && 1));
		_cc_x = 7;
		assert(_cc_x==7);
		rmw(__fence_var, 0);
		level7 = 0;
		}
}
void*t8(){
		while(1){
				level8 = 0;
		rmw(__fence_var, 0);
		waiting0=8;
		rmw(__fence_var, 0);
		assume(waiting0!=8 || (level0 < 0 && level1 < 0 && level2 < 0 && level3 < 0 && level4 < 0 && level5 < 0 && level6 < 0 && level7 < 0 && level9 < 0 && 1));
		level8 = 1;
		rmw(__fence_var, 0);
		waiting1=8;
		rmw(__fence_var, 0);
		assume(waiting1!=8 || (level0 < 1 && level1 < 1 && level2 < 1 && level3 < 1 && level4 < 1 && level5 < 1 && level6 < 1 && level7 < 1 && level9 < 1 && 1));
		level8 = 2;
		rmw(__fence_var, 0);
		waiting2=8;
		rmw(__fence_var, 0);
		assume(waiting2!=8 || (level0 < 2 && level1 < 2 && level2 < 2 && level3 < 2 && level4 < 2 && level5 < 2 && level6 < 2 && level7 < 2 && level9 < 2 && 1));
		level8 = 3;
		rmw(__fence_var, 0);
		waiting3=8;
		rmw(__fence_var, 0);
		assume(waiting3!=8 || (level0 < 3 && level1 < 3 && level2 < 3 && level3 < 3 && level4 < 3 && level5 < 3 && level6 < 3 && level7 < 3 && level9 < 3 && 1));
		level8 = 4;
		rmw(__fence_var, 0);
		waiting4=8;
		rmw(__fence_var, 0);
		assume(waiting4!=8 || (level0 < 4 && level1 < 4 && level2 < 4 && level3 < 4 && level4 < 4 && level5 < 4 && level6 < 4 && level7 < 4 && level9 < 4 && 1));
		level8 = 5;
		rmw(__fence_var, 0);
		waiting5=8;
		rmw(__fence_var, 0);
		assume(waiting5!=8 || (level0 < 5 && level1 < 5 && level2 < 5 && level3 < 5 && level4 < 5 && level5 < 5 && level6 < 5 && level7 < 5 && level9 < 5 && 1));
		level8 = 6;
		rmw(__fence_var, 0);
		waiting6=8;
		rmw(__fence_var, 0);
		assume(waiting6!=8 || (level0 < 6 && level1 < 6 && level2 < 6 && level3 < 6 && level4 < 6 && level5 < 6 && level6 < 6 && level7 < 6 && level9 < 6 && 1));
		level8 = 7;
		rmw(__fence_var, 0);
		waiting7=8;
		rmw(__fence_var, 0);
		assume(waiting7!=8 || (level0 < 7 && level1 < 7 && level2 < 7 && level3 < 7 && level4 < 7 && level5 < 7 && level6 < 7 && level7 < 7 && level9 < 7 && 1));
		level8 = 8;
		rmw(__fence_var, 0);
		waiting8=8;
		rmw(__fence_var, 0);
		assume(waiting8!=8 || (level0 < 8 && level1 < 8 && level2 < 8 && level3 < 8 && level4 < 8 && level5 < 8 && level6 < 8 && level7 < 8 && level9 < 8 && 1));
		_cc_x = 8;
		assert(_cc_x==8);
		rmw(__fence_var, 0);
		level8 = 0;
		}
}
void*t9(){
		while(1){
				level9 = 0;
		rmw(__fence_var, 0);
		waiting0=9;
		rmw(__fence_var, 0);
		assume(waiting0!=9 || (level0 < 0 && level1 < 0 && level2 < 0 && level3 < 0 && level4 < 0 && level5 < 0 && level6 < 0 && level7 < 0 && level8 < 0 && 1));
		level9 = 1;
		rmw(__fence_var, 0);
		waiting1=9;
		rmw(__fence_var, 0);
		assume(waiting1!=9 || (level0 < 1 && level1 < 1 && level2 < 1 && level3 < 1 && level4 < 1 && level5 < 1 && level6 < 1 && level7 < 1 && level8 < 1 && 1));
		level9 = 2;
		rmw(__fence_var, 0);
		waiting2=9;
		rmw(__fence_var, 0);
		assume(waiting2!=9 || (level0 < 2 && level1 < 2 && level2 < 2 && level3 < 2 && level4 < 2 && level5 < 2 && level6 < 2 && level7 < 2 && level8 < 2 && 1));
		level9 = 3;
		rmw(__fence_var, 0);
		waiting3=9;
		rmw(__fence_var, 0);
		assume(waiting3!=9 || (level0 < 3 && level1 < 3 && level2 < 3 && level3 < 3 && level4 < 3 && level5 < 3 && level6 < 3 && level7 < 3 && level8 < 3 && 1));
		level9 = 4;
		rmw(__fence_var, 0);
		waiting4=9;
		rmw(__fence_var, 0);
		assume(waiting4!=9 || (level0 < 4 && level1 < 4 && level2 < 4 && level3 < 4 && level4 < 4 && level5 < 4 && level6 < 4 && level7 < 4 && level8 < 4 && 1));
		level9 = 5;
		rmw(__fence_var, 0);
		waiting5=9;
		rmw(__fence_var, 0);
		assume(waiting5!=9 || (level0 < 5 && level1 < 5 && level2 < 5 && level3 < 5 && level4 < 5 && level5 < 5 && level6 < 5 && level7 < 5 && level8 < 5 && 1));
		level9 = 6;
		rmw(__fence_var, 0);
		waiting6=9;
		rmw(__fence_var, 0);
		assume(waiting6!=9 || (level0 < 6 && level1 < 6 && level2 < 6 && level3 < 6 && level4 < 6 && level5 < 6 && level6 < 6 && level7 < 6 && level8 < 6 && 1));
		level9 = 7;
		rmw(__fence_var, 0);
		waiting7=9;
		rmw(__fence_var, 0);
		assume(waiting7!=9 || (level0 < 7 && level1 < 7 && level2 < 7 && level3 < 7 && level4 < 7 && level5 < 7 && level6 < 7 && level7 < 7 && level8 < 7 && 1));
		level9 = 8;
		rmw(__fence_var, 0);
		waiting8=9;
		rmw(__fence_var, 0);
		assume(waiting8!=9 || (level0 < 8 && level1 < 8 && level2 < 8 && level3 < 8 && level4 < 8 && level5 < 8 && level6 < 8 && level7 < 8 && level8 < 8 && 1));
		_cc_x = 9;
		assert(_cc_x==9);
		rmw(__fence_var, 0);
		level9 = 0;
		}
}
int main(){
	pthread_t _cs_tdiff0, _cs_tdiff1, _cs_tdiff2, _cs_tdiff3, _cs_tdiff4, _cs_tdiff5, _cs_tdiff6, _cs_tdiff7, _cs_tdiff8, _cs_tdiff9;
	pthread_create(&_cs_tdiff0, 0, t0, 0);
	pthread_create(&_cs_tdiff1, 0, t1, 0);
	pthread_create(&_cs_tdiff2, 0, t2, 0);
	pthread_create(&_cs_tdiff3, 0, t3, 0);
	pthread_create(&_cs_tdiff4, 0, t4, 0);
	pthread_create(&_cs_tdiff5, 0, t5, 0);
	pthread_create(&_cs_tdiff6, 0, t6, 0);
	pthread_create(&_cs_tdiff7, 0, t7, 0);
	pthread_create(&_cs_tdiff8, 0, t8, 0);
	pthread_create(&_cs_tdiff9, 0, t9, 0);
	return 0;
	}


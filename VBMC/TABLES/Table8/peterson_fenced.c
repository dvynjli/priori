 #include <pthread.h>
#define assert(e) if (!(e)) ERROR: goto ERROR;

int flag1 = 0, flag2 = 0;
int turn;
int _cc_x;

int __fence_var;
void *t0()
{
    int rflag2 = -1, rturn = -1, rx = -1;
    while(1){
    flag1 = 1;
    rmw(__fence_var, 0);
    turn = 1;
    rmw(__fence_var, 0);
    rflag2 = flag2;
    rturn = turn;
    assume(rflag2!=1 || rturn!=1);
    _cc_x = 0;
    rx = _cc_x;
    assert(rx<=0);
    rmw(__fence_var, 0);
    flag1 = 0;
}
}

void *t1()
{
    int rflag1 = -1, rturn = -1, rx = -1;
    while(1){
    flag2 = 1;
    rmw(__fence_var, 0);
    turn = 0;
    rmw(__fence_var, 0);
    rflag1 = flag1;
    rturn = turn;
    assume(rflag1!=1 || rturn!=0);
    _cc_x = 1;
    rx = _cc_x;
    assert(rx>=1);
    rmw(__fence_var, 0);
    flag2 = 0;
}
}

int *propertyChecking(void* arg)
{
    return 0;
}

int main()
{
    pthread_t _cs_t1, _cs_t2;
    pthread_create(&_cs_t1, 0, t0, 0);
    pthread_create(&_cs_t2, 0, t1, 0);
    return 0;
}

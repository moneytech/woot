#include <sched.h>
#include <thread.h>

int sched_yield()
{
    Thread *ct = Thread::GetCurrent();
    if(ct) ct->Yield();
    return 0;
}

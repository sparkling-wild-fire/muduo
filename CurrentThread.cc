#include "CurrentThread.h"

namespace CurrentThread
{
    __thread int t_cachedTid = 0;   

    void cacheTid()
    {
        if (t_cachedTid == 0)  // 减少系统调用
        {
            // 通过linux系统调用，获取当前线程的tid值，这个是内核的tcb的id吧
            t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
        }
    }
}
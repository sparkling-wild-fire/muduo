#pragma once

#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread
{
    // 具体来说，当我们在一个 C++ 文件中使用另一个文件中定义的变量或函数时，需要使用 extern 关键字来声明该变量或函数。
    // 这样编译器就知道该变量或函数的定义在其他文件中，不需要在当前文件中分配存储空间。
    extern __thread int t_cachedTid;   // 这个extern干嘛的了

    void cacheTid();

    inline int tid()    // 为什么不直接用cacheTid
    {
        if (__builtin_expect(t_cachedTid == 0, 0))
        {
            cacheTid();
        }
        return t_cachedTid;
    }
}
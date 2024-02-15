#include "Buffer.h"

#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>

/**
 * 从fd上读取数据  Poller工作在LT模式
 * Buffer缓冲区是有大小的！ 但是从fd上读数据的时候，却不知道tcp数据最终的大小  =》 缓存区大小不好分配，太小了放不下，太大了浪费空间 =》 弄一块大内存的栈空间
 */

// 读fd，就是把数据写到我的可写缓存区
// 写fd，就是把我可读缓存区的数据发送到fd  那我可读缓存区的数据是谁写的呢？


// 其实就是一段数据，可读缓存区是有数据的部分，可写缓存区是没数据的部分
ssize_t Buffer::readFd(int fd, int* saveErrno)
{
    char extrabuf[65536] = {0}; // 栈上的内存空间  64K  =》 分配快，自动回收   =》 那我调用完了，栈被释放了，我的缓存数据也被释放了啊
    
    struct iovec vec[2];
    
    const size_t writable = writableBytes(); // 这是Buffer底层缓冲区剩余的可写空间大小
    vec[0].iov_base = begin() + writerIndex_;   // 第一块缓冲区
    vec[0].iov_len = writable;

    vec[1].iov_base = extrabuf;    // 第二块缓冲区    =》 扩容时再把这个缓存区的数据写buff？  => 对的
    vec[1].iov_len = sizeof extrabuf;
    
    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;   // buff可写长度小于64k，写第二块缓冲区，否则写buff  => 一次最多读64k
    const ssize_t n = ::readv(fd, vec, iovcnt);  // readv函数支持多块缓存区的读写
    if (n < 0)
    {
        *saveErrno = errno;
    }
    else if (n <= writable) // Buffer的可写缓冲区已经够存储读出来的数据了
    {
        writerIndex_ += n;
    }
    else // extrabuf里面也写入了数据 
    {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);  // writerIndex_开始写 n - writable大小的数据
    }

    return n;
}

ssize_t Buffer::writeFd(int fd, int* saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0)
    {
        *saveErrno = errno;
    }
    return n;
}
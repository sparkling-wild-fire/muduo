#include "EventLoopThread.h"
#include "EventLoop.h"


EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, 
        const std::string &name)
        : loop_(nullptr)
        , exiting_(false)
        , thread_(std::bind(&EventLoopThread::threadFunc, this), name)   // this
        , mutex_()
        , cond_()
        , callback_(cb)
{

}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop()
{
    thread_.start(); // 启动底层的新线程，执行下面的方法

    EventLoop *loop = nullptr;   // 得等下面得函数把loop初始化好了后，才继续执行（才让别人访问这个线程的loop对象）
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while ( loop_ == nullptr )
        {
            cond_.wait(lock);          // 等子线程创建完成loop后，才往下走
        }
        loop = loop_;
    }
    return loop;
}

// 下面这个方法，是在单独的新线程里面运行的
void EventLoopThread::threadFunc()
{
    // 难道时主线程创建的loop?   入口函数
    EventLoop loop; // 创建一个独立的eventloop，和上面的线程是一一对应的，one loop per thread

    if (callback_)
    {
        callback_(&loop);  // 把loop传给你这个回调函数
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);  // 这里为什么要加锁？  多个线程给loop赋值？ loop不就是个线程局部变量嘛  =》 不能同时对loop进行修改？  => 看下这个线程对象怎么初始化的吧
        loop_ = &loop;
        cond_.notify_one();  // 通知主线程，要这个loop创建完成后，才能被你追加到epoll向量中
    }

    loop.loop();               // EventLoop loop  => Poller.poll
    std::unique_lock<std::mutex> lock(mutex_);   // 当这个loop退出后，将loop_置为空
    loop_ = nullptr;
}
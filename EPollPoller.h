#pragma once

#include "Poller.h"
#include "Timestamp.h"

#include <vector>
#include <sys/epoll.h>

class Channel;

/**
 * epoll的使用  
 * epoll_create
 * epoll_ctl   add/mod/del
 * epoll_wait
 */ 
class EPollPoller : public Poller
{
public:
    EPollPoller(EventLoop *loop);
    ~EPollPoller() override;

    // 重写基类Poller的抽象方法， epoll_wait在poll里面？
    // 只用了ChannelList *指针类型，只需要在前面声明就好了
    Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;
private:
    static const int kInitEventListSize = 16;

    // 填写活跃的连接
    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
    // 更新channel通道
    void update(int operation, Channel *channel);

    using EventList = std::vector<epoll_event>;

    // epollfd 是一个 epoll 实例的文件描述符，它是通过调用 epoll_create() 或者 epoll_create1() 函数创建的。
    // epollfd 可以用于向内核注册需要监控的文件描述符，以及获取内核通知的事件。在使用完 epoll 后，需要使用 close() 函数关闭 epollfd 文件描述符，以释放资源。
    int epollfd_;
    EventList events_;
};
#ifndef __EPOLL_EPOLL_DISPATCHER_H__
#define __EPOLL_EPOLL_DISPATCHER_H__

#include <common/exception.h>
#include <common/lockfree_queue.h>

namespace Epoll
{

class EventHandler;
class Dispatcher
{
public:
    Dispatcher();
    ~Dispatcher();
    void Stop();
    void Execute();
    void Register(uint32_t events, const EventHandler& handler);
    void Postpone(const std::function<void()>& task);

private:
    const int epollHandle_;
    std::atomic<bool> isStopped_;
    typedef Common::LockfreeBoundedQueue<std::function<void()> > TaskQueue;
    TaskQueue postponedTasks_;
};

class EventHandler : private Common::Noncopyable
{
public:
    EventHandler(Dispatcher& dispatcher);
    virtual ~EventHandler();

    void StartEventProcessing(int fd, uint32_t events);
    bool StopEventProcessing();
    int GetNativeHandle() const;
    Dispatcher& GetDispatcher();
    const Dispatcher& GetDispatcher() const;

    void HandleEpollEvents(uint32_t events);

protected:
    virtual void HandleReadEvent() = 0;
    virtual void HandleWriteEvent() = 0;
    virtual void HandleErrorEvent() = 0;

private:
    Dispatcher& dispatcher_;
    std::atomic<int> fd_;
};

} // namespace Epoll

#endif // __EPOLL_EPOLL_DISPATCHER_H__

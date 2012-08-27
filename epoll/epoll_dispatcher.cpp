#include "epoll_dispatcher.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>

namespace Epoll
{

Dispatcher::Dispatcher()
    : epollHandle_(epoll_create1(0))
    , isStopped_(false)
    , postponedTasks_(1024)
{
    if (-1 == epollHandle_)
        throw Common::PlatformException(errno, "Dispatcher: cannot create epoll handle by epoll_create1");
}

Dispatcher::~Dispatcher()
{
    Stop();
}

void Dispatcher::Stop()
{
    bool expected = false;
    if (isStopped_.compare_exchange_weak(expected, true))
    {
        if (-1 == close(epollHandle_))
            throw Common::PlatformException(errno, "Dispatcher: cannot close descriptor");
    }
}

void Dispatcher::Execute()
{
    epoll_event events[128];
    while (!isStopped_)
    {
        const int eventNumber = epoll_wait(epollHandle_, events, sizeof(events)/sizeof(events[0]), -1);

        if (-1 == eventNumber && EINTR != errno)
            throw Common::PlatformException(errno, "Dispatcher: unexpected error on epoll_wait");

        for (int i = 0; i < eventNumber; ++i)
        {
            try
            {
                reinterpret_cast<EventHandler*>(events[i].data.ptr)->HandleEpollEvents(events[i].events);
            }
            catch (...)
            {
                Common::LogException();
            }
        }

        std::function<void()> postponedTask;
        while (postponedTasks_.TryPop(postponedTask));
    }
}

void Dispatcher::Register(uint32_t events, const EventHandler& handler)
{
    epoll_event event = {events, { const_cast<void*>(reinterpret_cast<const void*>(&handler)) } };
    if (-1 == epoll_ctl(epollHandle_, EPOLL_CTL_ADD, handler.GetNativeHandle(), &event))
        throw Common::PlatformException(errno, "Dispatcher: cannot register socket by epoll_ctl");
}

void Dispatcher::Postpone(const std::function<void()>& task)
{
    postponedTasks_.TryPush(task);
}

EventHandler::EventHandler(Dispatcher& dispatcher) : dispatcher_(dispatcher), fd_(0)
{
}

EventHandler::~EventHandler()
{
    StopEventProcessing();
}

void EventHandler::StartEventProcessing(int fd, uint32_t events)
{
    if (fd_)
        throw Common::Exception(0, "EventHandler: double start");
    if (!(fd_ = fd))
        throw Common::Exception(0, "EventHandler: empty descriptor");
    dispatcher_.Register(events, *this);
}

bool EventHandler::StopEventProcessing()
{
    int fd = fd_;
    if (fd && fd_.compare_exchange_weak(fd, 0))
    {
        shutdown(fd, SHUT_RDWR);
        close(fd);
        return true;
    }
    return false;
}

int EventHandler::GetNativeHandle() const
{
    return fd_;
}

Dispatcher& EventHandler::GetDispatcher()
{
    return dispatcher_;
}
const Dispatcher& EventHandler::GetDispatcher() const
{
    return dispatcher_;
}


void EventHandler::HandleEpollEvents(uint32_t events)
{
    if (events & EPOLLIN)
    {
        HandleReadEvent();
    }
    else if (events & EPOLLOUT)
    {
        HandleWriteEvent();
    }
    else
    {
        assert((events & (EPOLLHUP | EPOLLERR)) && "unknown event");
        HandleErrorEvent();
    }
}


} // namespace Epoll

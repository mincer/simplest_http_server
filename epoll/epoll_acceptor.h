#ifndef __EPOLL_EPOLL_ACCEPTOR_H__
#define __EPOLL_EPOLL_ACCEPTOR_H__

#include "epoll_socket.h"
#include <atomic>
#include <memory>

namespace Common
{
class BoundedTaskExecutor;
} // namespace Common

namespace Epoll
{

class Dispatcher;

class Acceptor : private EventHandler, public std::enable_shared_from_this<Acceptor>
{
public:
    typedef std::function<void (int error, Socket::Ptr socket)> AcceptHandler;
    typedef std::shared_ptr<Acceptor> Ptr;

    static Ptr Create(Dispatcher& dispatcher, Common::BoundedTaskExecutor& executor, const std::string& port);
    ~Acceptor();

    void AsyncAccept(AcceptHandler handler);
    void Close();

protected:
    virtual void HandleReadEvent();
    virtual void HandleWriteEvent();
    virtual void HandleErrorEvent();

private:
    Acceptor(Dispatcher& dispatcher, Common::BoundedTaskExecutor& executor);
    void Start(const std::string& port);

    void HandleAccept();

private:
    Common::BoundedTaskExecutor& executor_;

    struct EventData
    {
        EventData() : canBeHandled(false), isRunning(0) {}

        AcceptHandler handler;
        std::atomic<bool> canBeHandled;
        std::atomic<uint32_t> isRunning;
    };
    EventData eventData_;
};

} // namespace Epoll

#endif // __EPOLL_EPOLL_ACCEPTOR_H__

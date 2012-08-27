#ifndef __EPOLL_EPOLL_SOCKET_H__
#define __EPOLL_EPOLL_SOCKET_H__

#include "epoll_dispatcher.h"
#include <memory>

namespace Common
{
class BoundedTaskExecutor;
} // namespace Common

namespace Epoll
{

class Dispatcher;

class Socket : private EventHandler, public std::enable_shared_from_this<Socket>
{
public:
    typedef std::function<void (int error, std::size_t bytesTransferred)> CompletionHandler;
    typedef std::shared_ptr<Socket> Ptr;

    ~Socket();

    static Ptr Create(Dispatcher& dispatcher, Common::BoundedTaskExecutor& executor, int fd);

    void Close();
    bool IsClosed() const;

    void AsyncRead(void* buffer, std::size_t size, CompletionHandler handler);
    void AsyncWrite(const void* buffer, std::size_t size, CompletionHandler handler);

protected:
    virtual void HandleReadEvent();
    virtual void HandleWriteEvent();
    virtual void HandleErrorEvent();

private:
    Socket(Dispatcher& dispatcher, Common::BoundedTaskExecutor& executor);
    void Start(int fd);

    void HandleReadEventInternal();
    void HandleWriteEventInternal();

private:
    Common::BoundedTaskExecutor& executor_;

    struct EventData
    {
        EventData() : buffer(nullptr), size(0), canBeHandled(false), isRunning(0) {}

        CompletionHandler handler;
        void* buffer;
        std::size_t size;
        std::atomic<bool> canBeHandled;
        std::atomic<uint32_t> isRunning;
    };

    EventData readData_;
    EventData writeData_;
};

} // namespace Epoll

#endif // __EPOLL_EPOLL_SOCKET_H__

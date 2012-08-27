#include "epoll_socket.h"
#include "epoll_common.h"
#include <common/task_executor.h>

#include <cassert>
#include <sys/epoll.h>
#include <sys/socket.h>

namespace Epoll
{

Socket::Ptr Socket::Create(Dispatcher& dispatcher, Common::BoundedTaskExecutor& executor, int fd)
{
    Ptr socket(new Socket(dispatcher, executor));
    socket->Start(fd);
    return socket;
}

Socket::Socket(Dispatcher& dispatcher, Common::BoundedTaskExecutor& executor)
    : EventHandler(dispatcher)
    , executor_(executor)
{
}

void Socket::Start(int fd)
{
    StartEventProcessing(fd, EPOLLIN | EPOLLOUT | EPOLLET);
}

Socket::~Socket()
{
    assert(IsClosed() && "it should be closed before");
}

void Socket::Close()
{
    if (StopEventProcessing())
    {
        HandleReadEventInternal();
        HandleWriteEventInternal();

        Ptr this_ = shared_from_this();
        GetDispatcher().Postpone([this_](){});
    }
}

bool Socket::IsClosed() const
{
    return !GetNativeHandle();
}

void Socket::AsyncRead(void* buffer, std::size_t size, CompletionHandler handler)
{
    assert(!readData_.handler && !readData_.canBeHandled);
    readData_.buffer = buffer;
    readData_.size = size;
    readData_.handler = handler;
    readData_.canBeHandled = true;
    HandleReadEventInternal();
}

void Socket::AsyncWrite(const void* buffer, std::size_t size, CompletionHandler handler)
{
    assert(!writeData_.handler && !writeData_.canBeHandled);
    writeData_.buffer = const_cast<void*>(buffer);
    writeData_.size = size;
    writeData_.handler = handler;
    writeData_.canBeHandled = true;
    HandleWriteEventInternal();
}

void Socket::HandleReadEvent()
{
    executor_.RunAsync(std::bind(&Socket::HandleReadEventInternal, shared_from_this()));
}

void Socket::HandleWriteEvent()
{
    executor_.RunAsync(std::bind(&Socket::HandleWriteEventInternal, shared_from_this()));
}

void Socket::HandleErrorEvent()
{
    HandleReadEvent();
    HandleWriteEvent();
}

void Socket::HandleReadEventInternal()
{
    if (++readData_.isRunning == 1)
    {
        do
        {
            if (readData_.canBeHandled)
            {
                const int fd = GetNativeHandle();
                if (!fd)
                {
                    InvokeHandler(readData_, ECONNABORTED, 0);
                    continue;
                }
                const ssize_t count = recv(fd, readData_.buffer, readData_.size, MSG_NOSIGNAL);
                if (count > 0)
                {
                    InvokeHandler(readData_, 0, count);
                }
                else if (-1 == count)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                        continue;

                    InvokeHandler(readData_, errno, 0);
                }
                else
                {
                    InvokeHandler(readData_, ECONNABORTED, 0);
                }
            }
        } while (--readData_.isRunning);
    }
}

void Socket::HandleWriteEventInternal()
{
    if (++writeData_.isRunning == 1)
    {
        do
        {
            if (writeData_.canBeHandled)
            {
                const int fd = GetNativeHandle();
                if (!fd)
                {
                    InvokeHandler(writeData_, ECONNABORTED, 0);
                    continue;
                }
                const ssize_t count = send(GetNativeHandle(), writeData_.buffer, writeData_.size, MSG_NOSIGNAL);
                if (count > 0)
                {
                    InvokeHandler(writeData_, 0, count);
                }
                else if (-1 == count)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                        continue;

                    InvokeHandler(writeData_, errno, 0);
                }
                else
                {
                    InvokeHandler(writeData_, ECONNABORTED, 0);
                }
            }
        } while (--writeData_.isRunning);
    }
}

} // namespace Epoll

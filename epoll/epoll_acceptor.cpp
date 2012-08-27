#include "epoll_acceptor.h"
#include "epoll_common.h"

#include <common/task_executor.h>
#include <string>

#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/epoll.h>

namespace Epoll
{

namespace
{

void ConfigureSocket(int fd)
{
    const int currentFlags = fcntl(fd, F_GETFL, 0);
    if (-1 == currentFlags)
        throw Common::PlatformException(errno, "fcntl");

    if (-1 == fcntl(fd, F_SETFL, currentFlags | O_NONBLOCK))
        throw Common::PlatformException(errno, "fcntl");
}

int CreateServerSocket(const std::string& port)
{
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    addrinfo* localAddresses = nullptr;
    if (int error = getaddrinfo(nullptr, port.c_str(), &hints, &localAddresses))
        throw Common::Exception(error, "cannot getaddrinfo");
    auto localAddressesGuard = Common::MakeGuard(localAddresses, &freeaddrinfo);
    (void)localAddressesGuard;

    addrinfo* address = nullptr;
    int boundSocket = 0;
    for (address = localAddresses; address != nullptr; address = address->ai_next)
    {
        boundSocket = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
        if (-1 == boundSocket)
            continue;
        auto boundSocketGuard = Common::MakeGuard(&boundSocket, [](int* fd){ close(*fd); });
        (void)boundSocketGuard;

        int value = 1;
        if (-1 == setsockopt(boundSocket, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)))
            throw Common::PlatformException(errno, "cannot set SO_REUSEADDR");

        if (-1 == setsockopt(boundSocket, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(value)))
            throw Common::PlatformException(errno, "cannot set SO_KEEPALIVE");

        if (0 == bind(boundSocket, address->ai_addr, address->ai_addrlen))
        {
            boundSocketGuard.release(); // socket is bound successfully
            break;
        }
    }

    if (!address)
        throw Common::Exception(0, "cannot bind address");

    ConfigureSocket(boundSocket);

    if (-1 == listen(boundSocket, SOMAXCONN))
        throw Common::Exception(0, "listen");

    return boundSocket;
}

} // namespace


Acceptor::Ptr Acceptor::Create(Dispatcher& dispatcher, Common::BoundedTaskExecutor& executor, const std::string& port)
{
    Ptr acceptor(new Acceptor(dispatcher, executor));
    acceptor->Start(port);
    return acceptor;
}

Acceptor::Acceptor(Dispatcher& dispatcher, Common::BoundedTaskExecutor& executor)
    : EventHandler(dispatcher)
    , executor_(executor)
{
}

void Acceptor::Start(const std::string& port)
{
    StartEventProcessing(CreateServerSocket(port), EPOLLIN | EPOLLET);
}

Acceptor::~Acceptor()
{
    Close();
}

void Acceptor::Close()
{
    if (StopEventProcessing())
    {
        HandleAccept();
        Ptr this_ = shared_from_this();
        GetDispatcher().Postpone([this_](){});
    }
}

void Acceptor::AsyncAccept(AcceptHandler handler)
{
    assert(!eventData_.handler && !eventData_.canBeHandled);
    eventData_.handler = handler;
    eventData_.canBeHandled = true;
    HandleAccept();
}

void Acceptor::HandleReadEvent()
{
    executor_.RunAsync(std::bind(&Acceptor::HandleAccept, shared_from_this()));
}

void Acceptor::HandleWriteEvent()
{
    assert(!"unexpected event");
}

void Acceptor::HandleErrorEvent()
{
    HandleReadEvent();
}

void Acceptor::HandleAccept()
{
    if (++eventData_.isRunning == 1)
    {
        do
        {
            if (eventData_.canBeHandled)
            {
                const int fd = GetNativeHandle();
                if (!fd)
                {
                    InvokeHandler(eventData_, ECONNABORTED, Socket::Ptr());
                    continue;
                }
                sockaddr incomingAddress;
                socklen_t incomingAddressLength = sizeof(incomingAddress);
                const int acceptedSocket = accept(fd, &incomingAddress, &incomingAddressLength);
                if (acceptedSocket > 0)
                {
                    ConfigureSocket(acceptedSocket);
                    InvokeHandler(eventData_, 0, Socket::Create(GetDispatcher(), executor_, acceptedSocket));
                }
                else if (-1 == acceptedSocket)
                {
                    if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
                        continue;

                    if (EMFILE == errno || ENFILE == errno)
                    {
                        // TODO: async timeout
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                        ++eventData_.isRunning;
                        continue;
                    }
                    InvokeHandler(eventData_, errno, Socket::Ptr());
                }
                else
                {
                    InvokeHandler(eventData_, ECONNABORTED, Socket::Ptr());
                }
            }
        } while (--eventData_.isRunning);
    }
}

} // namespace Epoll

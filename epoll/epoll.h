#ifndef __EPOLL_EPOLL_H__
#define __EPOLL_EPOLL_H__

#include "epoll_acceptor.h"
#include "epoll_dispatcher.h"
#include "epoll_socket.h"

namespace Epoll
{

struct ConnectionTraits
{
    typedef Epoll::Dispatcher Dispatcher;
    typedef Epoll::Socket Socket;
    typedef Epoll::Acceptor Acceptor;
};

} // namespace Epoll

#endif // __EPOLL_EPOLL_H__

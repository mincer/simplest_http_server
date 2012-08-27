#ifndef __EPOLL_EPOLL_COMMON_H__
#define __EPOLL_EPOLL_COMMON_H__

namespace Epoll
{

template<typename TEventData, typename T1, typename T2>
inline void InvokeHandler(TEventData& eventData, const T1& t1, const T2& t2)
{
    eventData.canBeHandled = false;
    decltype(eventData.handler) tmpHandler;
    eventData.handler.swap(tmpHandler);
    tmpHandler(t1, t2);
}


} // namespace Epoll

#endif // __EPOLL_EPOLL_COMMON_H__

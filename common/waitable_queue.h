#ifndef __COMMON_WAITABLE_QUEUE_H__
#define __COMMON_WAITABLE_QUEUE_H__

#include "lockfree_queue.h"
#include <atomic>
#include <condition_variable>
#include <mutex>

namespace Common
{

template<typename T>
class WaitableBoundedQueue
{
public:
    WaitableBoundedQueue(std::size_t maxSize) : isStopped_(false), queue_(maxSize)
    {
    }

    void Stop()
    {
        isStopped_ = true;
        queueNotEmpty_.notify_all();
        queueNotFull_.notify_all();
    }

    bool TryPush(const T& value)
    {
        if (queue_.TryPush(value))
        {
            queueNotEmpty_.notify_one();
            return true;
        }
        return false;

    }

    bool TryPop(T& value)
    {
        if (queue_.TryPop(value) && isStopped_ == false)
        {
            queueNotFull_.notify_one();
            return true;
        }
        return false;
    }

    bool Push(const T& value)
    {
        while (!queue_.TryPush(value) && isStopped_ == false)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            queueNotFull_.wait(lock);
        }
        if (isStopped_)
            return false;
        queueNotEmpty_.notify_one();
        return true;
    }

    bool Pop(T& value)
    {
        while (!queue_.TryPop(value) && isStopped_ == false)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            queueNotEmpty_.wait(lock);
        }
        if (isStopped_)
            return false;
        queueNotFull_.notify_one();
        return true;
    }

private:
    std::atomic<bool> isStopped_;
    Common::LockfreeBoundedQueue<T> queue_;
    std::mutex mutex_;
    std::condition_variable queueNotFull_;
    std::condition_variable queueNotEmpty_;
};

} // namespace Common

#endif // __COMMON_WAITABLE_QUEUE_H__

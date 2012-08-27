#ifndef __COMMON_TASK_EXECUTOR_H__
#define __COMMON_TASK_EXECUTOR_H__

#include "waitable_queue.h"
#include <algorithm>
#include <atomic>
#include <memory>
#include <thread>
#include <vector>

namespace Common
{

typedef std::function<void ()> Task;
class BoundedTaskExecutor
{
public:
    BoundedTaskExecutor(std::size_t maxTaskQueueSize = 1024)
        : queue_(maxTaskQueueSize)
        , isStopped_(true)
    {
    }

    ~BoundedTaskExecutor()
    {
        Stop();
        WaitToStop();
    }

    void Stop()
    {
        isStopped_ = true;
        queue_.Stop();
    }

    void RunAsync(const Task& task)
    {
        queue_.Push(task);
    }

    void ExecuteThreadPool(std::size_t threadCount)
    {
        isStopped_ = false;
        threads_.resize(threadCount);
        std::generate(std::begin(threads_), std::end(threads_), [&]{ return std::thread(std::bind(&BoundedTaskExecutor::ThreadFunction, this)); });
    }

    void WaitToStop()
    {
        std::for_each(std::begin(threads_), std::end(threads_), std::bind(&std::thread::join, std::placeholders::_1));
    }

private:
    void ThreadFunction()
    {
        Task task;
        while (queue_.Pop(task) && !isStopped_)
        {
            try
            {
                assert(task);
                task();
                task = Task();
            }
            catch (...)
            {
                LogException();
            }
        }
    }

private:
    typedef WaitableBoundedQueue<Task> TaskQueue;
    TaskQueue queue_;
    std::atomic<bool> isStopped_;
    std::vector<std::thread> threads_;
};

} // namespace Common

#endif // #ifndef __COMMON_TASK_EXECUTOR_H__


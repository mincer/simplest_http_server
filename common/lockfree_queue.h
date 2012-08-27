#ifndef __COMMON_LOCKFREE_BOUNDED_QUEUE_H__
#define __COMMON_LOCKFREE_BOUNDED_QUEUE_H__

#include "noncopyable.h"
#include <atomic>
#include <cassert>

namespace Common
{

template<typename T>
class LockfreeBoundedQueue : private Noncopyable
{
public:
    LockfreeBoundedQueue(std::size_t bufferMaxSize)
        : buffer_(new Element[bufferMaxSize])
        , bufferMask_(bufferMaxSize - 1)
    {
        assert((bufferMaxSize >= 2) && ((bufferMaxSize & (bufferMaxSize - 1)) == 0));
        for (std::size_t i = 0; i != bufferMaxSize; i += 1)
        {
          buffer_[i].sequence_.store(i, std::memory_order_relaxed);
        }
        backPos_.store(0, std::memory_order_relaxed);
        frontPos_.store(0, std::memory_order_relaxed);
    }

    ~LockfreeBoundedQueue()
    {
        delete [] buffer_;
    }

    bool TryPush(const T& value)
    {
        Element* element;
        std::size_t pos = backPos_.load(std::memory_order_relaxed);
        for (;;)
        {
            element = &buffer_[pos & bufferMask_];
            const std::size_t seq = element->sequence_.load(std::memory_order_acquire);
            const intptr_t dif = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos);
            if (dif == 0)
            {
                if (backPos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
                    break;
            }
            else if (dif < 0)
            {
                return false;
            }
            else
            {
                pos = backPos_.load(std::memory_order_relaxed);
            }
        }
        element->value_ = value;
        element->sequence_.store(pos + 1, std::memory_order_release);
        return true;
    }

    bool TryPop(T& value)
    {
        Element* element;
        std::size_t pos = frontPos_.load(std::memory_order_relaxed);
        for (;;)
        {
            element = &buffer_[pos & bufferMask_];
            const std::size_t seq = element->sequence_.load(std::memory_order_acquire);
            const intptr_t dif = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos + 1);
            if (dif == 0)
            {
                if (frontPos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
                  break;
            }
            else if (dif < 0)
            {
                return false;
            }
            else
            {
                pos = frontPos_.load(std::memory_order_relaxed);
            }
        }
        value = element->value_;
        element->value_ = T();
        element->sequence_.store(pos + bufferMask_ + 1, std::memory_order_release);
        return true;
    }

  private:
      struct Element
      {
          std::atomic<std::size_t> sequence_;
          T value_;
      };

      static const std::size_t CACHELINE_SIZE = 64;
      typedef char CachelinePad [CACHELINE_SIZE];

      CachelinePad pad0_;
      Element* const buffer_;
      const std::size_t bufferMask_;
      CachelinePad pad1_;
      std::atomic<std::size_t> backPos_;
      CachelinePad pad2_;
      std::atomic<std::size_t> frontPos_;
      CachelinePad pad3_;
};

} // namespace Common

#endif // __COMMON_LOCKFREE_BOUNDED_QUEUE_H__

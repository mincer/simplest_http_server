#ifndef __COMMON_BUFFER_H__
#define __COMMON_BUFFER_H__

#include "noncopyable.h"
#include <unordered_map>
#include <vector>
#include <mutex>

namespace Common
{

template<typename TKey, typename TData>
class Cache : private Common::Noncopyable
{
public:
    Cache(std::size_t maxSize) : maxSize_(maxSize) {}

    template<typename TGenerator>
    TData GetData(const TKey& key, const TGenerator& generator)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto result = currentCache_.insert(std::make_pair(key, TData()));
        if (result.second)
            result.first->second = generator();

        const TData data = result.first->second;
        if (currentCache_.size() > maxSize_/2)
        {
            nextCache_[key] = data;
            if (currentCache_.size() > maxSize_)
            {
                currentCache_.swap(nextCache_);
                nextCache_.clear();
            }
        }
        return data;
    }

private:
    const std::size_t maxSize_;
    typedef std::unordered_map<TKey, TData> CacheType;
    CacheType currentCache_;
    CacheType nextCache_;
    std::mutex mutex_;
};

template<typename T>
class ConstBuffer
{
public:
    ConstBuffer()
        : data(0)
        , size(0)
    {
    }
    ConstBuffer(const T* data, std::size_t size)
        : data(data)
        , size(size)
    {
    }
    ConstBuffer(const std::string& str)
        : data(str.data())
        , size(str.size())
    {
    }
    template<typename Type, std::size_t Size>
    ConstBuffer(const Type(&data)[Size])
        : data(data)
        , size(Size)
    {
    }

    const T* data;
    std::size_t size;
};

} // namespace Common

#endif // __COMMON_BUFFER_H__

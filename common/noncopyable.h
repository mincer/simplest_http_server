#ifndef __COMMON_NONCOPYABLE_H__
#define __COMMON_NONCOPYABLE_H__

namespace Common
{

class Noncopyable
{
protected:
    Noncopyable() {}
    ~Noncopyable() {}
private:
    Noncopyable(const Noncopyable&);
    const Noncopyable& operator=(const Noncopyable&);
};

} // namespace Common

#endif // __COMMON_NONCOPYABLE_H__

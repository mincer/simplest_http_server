#ifndef __COMMON_EXCEPTION_H_
#define __COMMON_EXCEPTION_H_

#include <exception>
#include <stdio.h>
#include <string.h>
#include <memory>

namespace Common
{

template<typename T, typename D>
std::unique_ptr<T, D> MakeGuard(T* t, D d)
{
    return std::unique_ptr<T, D>(t, d);
}

template<typename D>
std::unique_ptr<void, D> MakeGuard(D d)
{
    return std::unique_ptr<void, D>(&d, d);
}

class Exception : public std::exception
{
public:
    Exception(int code, const char* what) : code_(code), what_(what) {}
    virtual const char* what() const throw() { return what_; }
    int getCode() const { return code_; }
private:
    const int code_;
    const char* what_;
};

class PlatformException : public Exception
{
public:
    PlatformException(int code, const char* what) : Exception(code, what) {}
};

inline void LogException()
{
    try
    {
        throw;
    }
    catch (const PlatformException& e)
    {
        const char* str = e.what();
        fprintf(stderr, "platform exception: code=%d, %s: %s", e.getCode(), str ? str : "empty description", strerror(e.getCode()));
    }
    catch (const Exception& e)
    {
        const char* str = e.what();
        fprintf(stderr, "exception: code=%d, %s", e.getCode(), str ? str : "empty description");
    }
    catch (const std::exception& e)
    {
        const char* str = e.what();
        fprintf(stderr, "std exception: %s", str ? str : "empty description");
    }
    catch (...)
    {
        fprintf(stderr, "unknown exception");
    }
}

} // namespace Common

#endif // __COMMON_EXCEPTION_H_

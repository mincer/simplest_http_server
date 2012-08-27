#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#include <string>
#include <vector>
#include "header.h"

namespace Http
{

struct Request
{
    std::string method;
    std::string uri;
    int httpVersionMajor;
    int httpVersionMinor;
    std::vector<Header> headers;
};

} // namespace Http

#endif // __HTTP_REQUEST_H__

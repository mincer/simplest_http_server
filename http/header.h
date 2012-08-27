#ifndef __HTTP_HEADER_H__
#define __HTTP_HEADER_H__

#include <string>

namespace Http
{

struct Header
{
    std::string name;
    std::string value;
};

} // namespace Http

#endif // __HTTP_HEADER_H__

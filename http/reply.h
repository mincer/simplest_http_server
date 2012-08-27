#ifndef __HTTP_REPLY_H__
#define __HTTP_REPLY_H__

#include "header.h"
#include <common/buffer.h>
#include <memory>
#include <vector>

namespace Http
{

typedef Common::ConstBuffer<char> ConstBuffer;

struct Reply
{
    enum StatusType
    {
        Ok = 200,
        Created = 201,
        Accepted = 202,
        NoContent = 204,
        MultipleChoices = 300,
        MovedPermanently = 301,
        MovedTemporarily = 302,
        NotModified = 304,
        BadRequest = 400,
        Unauthorized = 401,
        Forbidden = 403,
        NotFound = 404,
        InternalServerError = 500,
        NotImplemented = 501,
        BadGateway = 502,
        ServiceUnavailable = 503
    };

    StatusType status;

    std::vector<Header> headers;

    struct Content
    {
        std::shared_ptr<char> data;
        std::size_t size;
    };
    Content content;

    std::vector<ConstBuffer> ToBuffers();
    void SetHeaders(std::size_t contentSize, const std::string& contentType);

    static Reply GetReply(StatusType status);
};

} // namespace Http

#endif // __HTTP_REPLY_H__

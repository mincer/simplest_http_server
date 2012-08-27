#ifndef __HTTP_REQUEST_HANDLER_H__
#define __HTTP_REQUEST_HANDLER_H__

#include "reply.h"
#include <common/buffer.h>
#include <common/noncopyable.h>
#include <string>

namespace Http
{

struct Reply;
struct Request;

class RequestHandler: private Common::Noncopyable
{
public:
    explicit RequestHandler(const std::string& documentRoot, std::size_t cacheSize);

    void HandleRequest(const Request& req, Reply& rep);

private:
    std::shared_ptr<std::vector<unsigned char> > GetContent();

private:
    const std::string documentRoot_;
    Common::Cache<std::string, Reply::Content> contentCache_;
};

} // namespace Http

#endif // __HTTP_REQUEST_HANDLER_H__

#include "request_handler.h"

#include "mime_types.h"
#include "request.h"
#include <sstream>
#include <fstream>

namespace Http
{

namespace
{

bool IsAbsolutePath(const std::string& path)
{
    return !(path.empty() || path[0] != '/' || path.find("..") != std::string::npos);
}

std::string GetExtension(const std::string& path)
{
    const std::size_t lastSlashPos = path.find_last_of("/");
    const std::size_t lastDotPos = path.find_last_of(".");
    if (lastDotPos != std::string::npos && lastDotPos > lastSlashPos)
        return path.substr(lastDotPos + 1);
    return std::string();
}

bool DecodeUrl(const std::string& in, std::string& out)
{
    out.clear();
    out.reserve(in.size());
    for (std::size_t i = 0; i < in.size(); ++i)
    {
        if (in[i] == '%')
        {
            if (i + 3 <= in.size())
            {
                int value = 0;
                std::istringstream is(in.substr(i + 1, 2));
                if (is >> std::hex >> value)
                {
                    out += static_cast<char>(value);
                    i += 2;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        else if (in[i] == '+')
        {
            out += ' ';
        }
        else
        {
            out += in[i];
        }
    }
    return true;
}

Reply::Content ReadFile(std::ifstream& input)
{
    Reply::Content result;

    input.seekg(0, std::ios::end);
    result.size = input.tellg();
    input.seekg(0, std::ios::beg);

    result.data.reset(new char[result.size], [](char* data){ delete[] data; });
    input.read(result.data.get(), result.size);

    return result;
}

}

RequestHandler::RequestHandler(const std::string& documentRoot, std::size_t cacheSize)
    : documentRoot_(documentRoot)
    , contentCache_(cacheSize)
{
}

void RequestHandler::HandleRequest(const Request& request, Reply& reply)
{
    std::string requestPath;
    if (!DecodeUrl(request.uri, requestPath))
    {
        reply = Reply::GetReply(Reply::BadRequest);
        return;
    }

    if (!IsAbsolutePath(requestPath))
    {
        reply = Reply::GetReply(Reply::BadRequest);
        return;
    }

    if (requestPath[requestPath.size() - 1] == '/')
    {
        requestPath += "index.html";
    }

    const std::string fullPath = documentRoot_ + requestPath;
    std::ifstream inputStream(fullPath, std::ios::in | std::ios::binary);
    if (!inputStream)
    {
        reply = Reply::GetReply(Reply::NotFound);
        return;
    }

    reply.status = Reply::Ok;
    reply.content = contentCache_.GetData(fullPath, [&inputStream](){ return ReadFile(inputStream); });
    reply.SetHeaders(reply.content.size, MimeTypes::ExtensionToType(GetExtension(requestPath)));
}

} // namespace http

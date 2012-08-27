#include "reply.h"
#include <string>
#include <sstream>

namespace Http
{

namespace
{

template<class T>
std::string ToString(const T& obj)
{
    std::stringstream out;
    out << obj;
    return out.str();
}

namespace StatusText
{

const char Ok[] = "HTTP/1.0 200 OK\r\n";
const char Created[] = "HTTP/1.0 201 Created\r\n";
const char Accepted[] = "HTTP/1.0 202 Accepted\r\n";
const char NoContent[] = "HTTP/1.0 204 No Content\r\n";
const char MultipleChoices[] = "HTTP/1.0 300 Multiple Choices\r\n";
const char MovedPermanently[] = "HTTP/1.0 301 Moved Permanently\r\n";
const char MovedTemporarily[] = "HTTP/1.0 302 Moved Temporarily\r\n";
const char NotModified[] = "HTTP/1.0 304 Not Modified\r\n";
const char BadRequest[] = "HTTP/1.0 400 Bad Request\r\n";
const char Unauthorized[] = "HTTP/1.0 401 Unauthorized\r\n";
const char Forbidden[] = "HTTP/1.0 403 Forbidden\r\n";
const char NotFound[] = "HTTP/1.0 404 Not Found\r\n";
const char InternalServerError[] = "HTTP/1.0 500 Internal Server Error\r\n";
const char NotImplemented[] = "HTTP/1.0 501 Not Implemented\r\n";
const char BadGateway[] = "HTTP/1.0 502 Bad Gateway\r\n";
const char ServiceUnavailable[] = "HTTP/1.0 503 Service Unavailable\r\n";

const char Separator[] = { ':', ' ' };
const char CrLf[] = { '\r', '\n' };

ConstBuffer ToBuffer(Reply::StatusType status)
{
    switch (status)
    {
    case Reply::Ok:
        return ConstBuffer(Ok);
    case Reply::Created:
        return ConstBuffer(Created);
    case Reply::Accepted:
        return  ConstBuffer(Accepted);
    case Reply::NoContent:
        return  ConstBuffer(NoContent);
    case Reply::MultipleChoices:
        return  ConstBuffer(MultipleChoices);
    case Reply::MovedPermanently:
        return  ConstBuffer(MovedPermanently);
    case Reply::MovedTemporarily:
        return  ConstBuffer(MovedTemporarily);
    case Reply::NotModified:
        return  ConstBuffer(NotModified);
    case Reply::BadRequest:
        return  ConstBuffer(BadRequest);
    case Reply::Unauthorized:
        return  ConstBuffer(Unauthorized);
    case Reply::Forbidden:
        return  ConstBuffer(Forbidden);
    case Reply::NotFound:
        return  ConstBuffer(NotFound);
    case Reply::InternalServerError:
        return  ConstBuffer(InternalServerError);
    case Reply::NotImplemented:
        return  ConstBuffer(NotImplemented);
    case Reply::BadGateway:
        return  ConstBuffer(BadGateway);
    case Reply::ServiceUnavailable:
        return  ConstBuffer(ServiceUnavailable);
    }
    return  ConstBuffer(InternalServerError);
}

} // namespace StatusText

namespace Replies
{

const char Ok[] = "";
const char Created[] = "<html>"
        "<head><title>Created</title></head>"
        "<body><h1>201 Created</h1></body>"
        "</html>";
const char Accepted[] = "<html>"
        "<head><title>Accepted</title></head>"
        "<body><h1>202 Accepted</h1></body>"
        "</html>";
const char NoContent[] = "<html>"
        "<head><title>No Content</title></head>"
        "<body><h1>204 Content</h1></body>"
        "</html>";
const char MultipleChoices[] = "<html>"
        "<head><title>Multiple Choices</title></head>"
        "<body><h1>300 Multiple Choices</h1></body>"
        "</html>";
const char MovedPermanently[] = "<html>"
        "<head><title>Moved Permanently</title></head>"
        "<body><h1>301 Moved Permanently</h1></body>"
        "</html>";
const char MovedTemporarily[] = "<html>"
        "<head><title>Moved Temporarily</title></head>"
        "<body><h1>302 Moved Temporarily</h1></body>"
        "</html>";
const char NotModified[] = "<html>"
        "<head><title>Not Modified</title></head>"
        "<body><h1>304 Not Modified</h1></body>"
        "</html>";
const char BadRequest[] = "<html>"
        "<head><title>Bad Request</title></head>"
        "<body><h1>400 Bad Request</h1></body>"
        "</html>";
const char Unauthorized[] = "<html>"
        "<head><title>Unauthorized</title></head>"
        "<body><h1>401 Unauthorized</h1></body>"
        "</html>";
const char Forbidden[] = "<html>"
        "<head><title>Forbidden</title></head>"
        "<body><h1>403 Forbidden</h1></body>"
        "</html>";
const char NotFound[] = "<html>"
        "<head><title>Not Found</title></head>"
        "<body><h1>404 Not Found</h1></body>"
        "</html>";
const char InternalServerError[] = "<html>"
        "<head><title>Internal Server Error</title></head>"
        "<body><h1>500 Internal Server Error</h1></body>"
        "</html>";
const char NotImplemented[] = "<html>"
        "<head><title>Not Implemented</title></head>"
        "<body><h1>501 Not Implemented</h1></body>"
        "</html>";
const char BadGateway[] = "<html>"
        "<head><title>Bad Gateway</title></head>"
        "<body><h1>502 Bad Gateway</h1></body>"
        "</html>";
const char ServiceUnavailable[] = "<html>"
        "<head><title>Service Unavailable</title></head>"
        "<body><h1>503 Service Unavailable</h1></body>"
        "</html>";

ConstBuffer ToBuffer(Reply::StatusType status)
{
    switch (status)
    {
    case Reply::Ok:
        return ConstBuffer(Ok);
    case Reply::Created:
        return ConstBuffer(Created);
    case Reply::Accepted:
        return ConstBuffer(Accepted);
    case Reply::NoContent:
        return ConstBuffer(NoContent);
    case Reply::MultipleChoices:
        return ConstBuffer(MultipleChoices);
    case Reply::MovedPermanently:
        return ConstBuffer(MovedPermanently);
    case Reply::MovedTemporarily:
        return ConstBuffer(MovedTemporarily);
    case Reply::NotModified:
        return ConstBuffer(NotModified);
    case Reply::BadRequest:
        return ConstBuffer(BadRequest);
    case Reply::Unauthorized:
        return ConstBuffer(Unauthorized);
    case Reply::Forbidden:
        return ConstBuffer(Forbidden);
    case Reply::NotFound:
        return ConstBuffer(NotFound);
    case Reply::InternalServerError:
        return ConstBuffer(InternalServerError);
    case Reply::NotImplemented:
        return ConstBuffer(NotImplemented);
    case Reply::BadGateway:
        return ConstBuffer(BadGateway);
    case Reply::ServiceUnavailable:
        return ConstBuffer(ServiceUnavailable);
    }
    return ConstBuffer(InternalServerError);
}

} // namespace Replies

} // namespace

std::vector<ConstBuffer> Reply::ToBuffers()
{
    std::vector<ConstBuffer> buffers;
    buffers.push_back(StatusText::ToBuffer(status));
    for (std::size_t i = 0; i < headers.size(); ++i)
    {
        Header& h = headers[i];
        buffers.push_back(h.name);
        buffers.push_back(StatusText::Separator);
        buffers.push_back(h.value);
        buffers.push_back(StatusText::CrLf);
    }
    buffers.push_back(StatusText::CrLf);
    if (content.data)
        buffers.push_back(ConstBuffer(content.data.get(), content.size));
    return buffers;
}

void Reply::SetHeaders(std::size_t contentSize, const std::string& contentType)
{
    headers.resize(3);
    headers[0].name = "Server";
    headers[0].value = "Simplest-HTTP-Server";
    headers[1].name = "Content-Length";
    headers[1].value = ToString(contentSize);
    headers[2].name = "Content-Type";
    headers[2].value = contentType;
}

Reply Reply::GetReply(Reply::StatusType status)
{
    Reply reply;
    reply.status = status;
    const ConstBuffer& statusBuffer = Replies::ToBuffer(status);
    reply.content.data.reset(const_cast<char*>(statusBuffer.data), [](const char*){} );
    reply.content.size = statusBuffer.size;
    reply.SetHeaders(reply.content.size, "text/html");
    return reply;
}

} // namespace http

#ifndef __HTTP_HTTP_CONNECTION_H__
#define __HTTP_HTTP_CONNECTION_H__

#include "http_acceptor.h"
#include "reply.h"
#include "request.h"
#include "request_handler.h"
#include "request_parser.h"
#include "transfer.h"

#include <memory>

namespace Http
{

template<typename Traits>
class Connection : public std::enable_shared_from_this<Connection<Traits> >
{
public:
    typedef std::shared_ptr<Connection<Traits> > Ptr;

    static Ptr Create(const typename Traits::Socket::Ptr& socket, RequestHandler& handler)
    {
        Ptr connection(new Connection<Traits>(socket, handler));
        connection->StartReading();
        return connection;
    }

    ~Connection()
    {
        assert(socket_->IsClosed() && "it should be closed before destruction");
    }

    void Close()
    {
        socket_->Close();
    }

private:
    Connection(const typename Traits::Socket::Ptr& socket, RequestHandler& handler)
            : socket_(socket)
            , handler_(handler)
    {
    }

    void StartReading()
    {
        socket_->AsyncRead(buffer_
                         , sizeof(buffer_)
                         , std::bind(&Connection::ReadHandler
                                   , this->shared_from_this()
                                   , std::placeholders::_1
                                   , std::placeholders::_2));

    }

    void StartWriting()
    {
        WriteAllOperation<Traits>::Start(socket_
                                       , reply_.ToBuffers()
                                       , std::bind(&Connection::WriteHandler
                                                    , this->shared_from_this()
                                                 , std::placeholders::_1));
    }

    void ReadHandler(int error, std::size_t bytesTransferred)
    {
        if (error)
        {
            Close();
            return;
        }

        const RequestParser::Result result = parser_.Parse(request_, &buffer_[0], &buffer_[0] + bytesTransferred);
        if (result == RequestParser::Success)
        {
            handler_.HandleRequest(request_, reply_);
            StartWriting();
        }
        else if (result == RequestParser::Failed)
        {
            reply_ = Http::Reply::GetReply(Http::Reply::BadRequest);
            StartWriting();
        }
        else
        {
            StartReading();
        }
    }

    void WriteHandler(int /*error*/)
    {
        Close();
    }

private:
    char buffer_[8192];
    typename Traits::Socket::Ptr socket_;
    Http::RequestHandler& handler_;
    Http::RequestParser parser_;
    Http::Request request_;
    Http::Reply reply_;
};

} // namespace Http

#endif // __HTTP_HTTP_CONNECTION_H__

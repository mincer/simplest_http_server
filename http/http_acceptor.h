#ifndef __HTTP_HTTP_ACCEPTOR_H__
#define __HTTP_HTTP_ACCEPTOR_H__

#include "http_connection.h"

namespace Http
{

template<typename Traits>
class Acceptor : public std::enable_shared_from_this<Acceptor<Traits> >
{
public:
    typedef std::shared_ptr<Acceptor<Traits> > Ptr;
    static Ptr Create(const typename Traits::Acceptor::Ptr& acceptor
                    , RequestHandler& handler)
    {
        Ptr result(new Acceptor<Traits>(acceptor, handler));
        result->StartAccept();
        return result;
    }

    void Close()
    {
        acceptor_->Close();
    }

private:
    Acceptor(const typename Traits::Acceptor::Ptr& acceptor
           , RequestHandler& handler)
        : acceptor_(acceptor)
        , handler_(handler)
    {
    }

    void StartAccept()
    {
        acceptor_->AsyncAccept(std::bind(&Acceptor::AcceptHandler
                                       , this->shared_from_this()
                                       , std::placeholders::_1
                                       , std::placeholders::_2));
    }

    void AcceptHandler(int error, typename Traits::Socket::Ptr socket)
    {
        if (!error)
        {
            assert(socket);
            Connection<Traits>::Create(socket, handler_);
            StartAccept();
        }
        else
        {
            acceptor_->Close();
        }
    }

private:
    typename Traits::Acceptor::Ptr acceptor_;
    RequestHandler& handler_;
};

} // namespace Http

#endif // __HTTP_HTTP_ACCEPTOR_H__

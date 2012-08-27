#ifndef __HTTP_TRANSFER_H__
#define __HTTP_TRANSFER_H__

namespace Http
{

template<typename Traits>
class WriteAllOperation : public std::enable_shared_from_this<WriteAllOperation<Traits> >
{
public:
    typedef std::shared_ptr<WriteAllOperation<Traits> > Ptr;

    static Ptr Start(const typename Traits::Socket::Ptr& socket
                   , const std::vector<ConstBuffer>& buffers
                   , const typename Traits::Socket::CompletionHandler& task)
    {
        Ptr operation(new WriteAllOperation<Traits>(socket, buffers, task));
        operation->Write();
        return operation;
    }

private:
    WriteAllOperation(const typename Traits::Socket::Ptr& socket
                    , const std::vector<ConstBuffer>& buffers
                    , const typename Traits::Socket::CompletionHandler& task)
        : socket_(socket)
        , task_(task)
        , buffers_(buffers)
        , transferred_(0)
        , transferredInTotal_(0)
    {
        it_ = (std::begin(buffers_));
    }

    void Write()
    {
        for (;;)
        {
            if (it_ != std::end(buffers_))
            {
                if (transferred_ < it_->size)
                {
                    socket_->AsyncWrite(it_->data + transferred_
                                      , it_->size - transferred_
                                      , std::bind(&WriteAllOperation<Traits>::WriteHandler
                                                , this->shared_from_this()
                                                , std::placeholders::_1
                                                , std::placeholders::_2));
                }
                else
                {
                    transferredInTotal_ += transferred_;
                    transferred_ = 0;
                    ++it_;
                    continue;
                }
            }
            else
            {
                task_(0, transferredInTotal_);
            }
            break;
        }
    }

    void WriteHandler(int error, std::size_t bytes)
    {
        transferred_ += bytes;
        if (error)
            task_(error, transferredInTotal_);
        else
            Write();
    }

private:
    typename Traits::Socket::Ptr socket_;
    typename Traits::Socket::CompletionHandler task_;
    const std::vector<ConstBuffer> buffers_;
    std::vector<ConstBuffer>::const_iterator it_;
    std::size_t transferred_;
    std::size_t transferredInTotal_;
};

} // namespace Http

#endif // __HTTP_TRANSFER_H__

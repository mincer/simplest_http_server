#include <common/exception.h>
#include <common/task_executor.h>
#include <epoll/epoll.h>

#include <http/http_acceptor.h>
#include <http/http_connection.h>

#include <sstream>
#include <signal.h>

namespace
{

const char* GetApplicationName(const char* path)
{
    const char* name = path;
    for (const char* p = path; *p; ++p)
    {
        if (*p == '\\' || *p == '/')
            name = p + 1;
    }
    return name;
}

int ToNumber(const std::string& str)
{
    int result;
    std::istringstream(str) >> result;
    return result;
}

class Server
{
    typedef Epoll::ConnectionTraits Traits;
public:
    static Server& GetInstance();
    void Execute(const char* documentRoot
               , const char* port
               , std::size_t cacheSize
                 , std::size_t threadCount);

private:
    static void StopHandler(int);
    void Stop();
    void SetupSignals();

private:
    Common::BoundedTaskExecutor executor_;
    typename Traits::Dispatcher dispatcher_;
};

Server& Server::GetInstance()
{
    static Server instance;
    return instance;
}

void Server::Execute(const char* documentRoot, const char* port, std::size_t cacheSize, std::size_t threadCount)
{
    if (!documentRoot || !port)
        throw Common::Exception(0, "invalid arguments");

    SetupSignals();

    Http::RequestHandler handler(documentRoot, cacheSize);

    typedef Http::Acceptor<Traits> Acceptor;
    Acceptor::Create(Traits::Acceptor::Create(dispatcher_, executor_, port), handler);

    executor_.ExecuteThreadPool(threadCount);
    dispatcher_.Execute();

    executor_.WaitToStop();
}

void Server::StopHandler(int)
{
    GetInstance().Stop();
}

void Server::Stop()
{
    // TODO: graceful stop
    exit(0);
}

void Server::SetupSignals()
{
    struct sigaction sa;
    sa.sa_handler = &Server::StopHandler;
    sigemptyset (&sa.sa_mask);
    sa.sa_flags = 0;

    if (-1 == sigaction(SIGINT, &sa, nullptr))
        throw Common::PlatformException(errno, "cannot set sigaction");
    if (-1 == sigaction(SIGTERM, &sa, nullptr))
        throw Common::PlatformException(errno, "cannot set sigaction");
    if (-1 == sigaction(SIGHUP, &sa, nullptr))
        throw Common::PlatformException(errno, "cannot set sigaction");
}

}

int main(int argc, char* argv[])
{
    if (argc != 5)
    {
        printf("usage: %s <document root> <port> <file cache size> <thread count>\n", GetApplicationName(argv[0]));
        return -1;
    }

    try
    {
        Server::GetInstance().Execute(argv[1], argv[2], ToNumber(argv[3]), ToNumber(argv[4]));
        return 0;
    }
    catch (...)
    {
        Common::LogException();
    }
    return -1;
}


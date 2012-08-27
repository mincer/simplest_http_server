#ifndef __HTTP_REQUEST_PARSER_H__
#define __HTTP_REQUEST_PARSER_H__

namespace Http
{

struct Request;
class RequestParser
{
public:
    RequestParser();

    enum Result
    {
        Failed = 0,
        Success,
        Indeterminate
    };

    template<typename InputIterator>
    Result Parse(Request& request, InputIterator begin, InputIterator end)
    {
        while (begin != end)
        {
            const Result result = Consume(request, *begin++);
            if (result == Success || result == Failed)
                return result;
        }
        Result result = Indeterminate;
        return result;
    }

private:
    Result Consume(Request& req, char input);

    enum State
    {
        MethodStart,
        Method,
        Uri,
        HttpVersionH,
        HttpVersionT1,
        HttpVersionT2,
        HttpVersionP,
        HttpVersionSlash,
        HttpVersionMajorStart,
        HttpVersionMajor,
        HttpVersionMinorStart,
        HttpVersionMinor,
        ExpectingNewline1,
        HeaderLineStart,
        HeaderLws,
        HeaderName,
        SpaceBeforeHeaderValue,
        HeaderValue,
        ExpectingNewline2,
        ExpectingNewline3
    };

    State state_;
};

} // namespace Http

#endif // __HTTP_REQUEST_PARSER_H__

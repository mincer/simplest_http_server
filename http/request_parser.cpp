#include "request_parser.h"
#include "request.h"

namespace Http
{

namespace
{

bool IsChar(int c)
{
    return c >= 0 && c <= 127;
}

bool IsCtl(int c)
{
    return (c >= 0 && c <= 31) || (c == 127);
}

bool IsSpecialChar(int c)
{
    switch (c)
    {
    case '(':
    case ')':
    case '<':
    case '>':
    case '@':
    case ',':
    case ';':
    case ':':
    case '\\':
    case '"':
    case '/':
    case '[':
    case ']':
    case '?':
    case '=':
    case '{':
    case '}':
    case ' ':
    case '\t':
        return true;
    default:
        return false;
    }
}

bool IsDigit(int c)
{
    return c >= '0' && c <= '9';
}

} // namespace

RequestParser::RequestParser() : state_(MethodStart)
{
}

RequestParser::Result RequestParser::Consume(Request& request, char input)
{
    switch (state_)
    {
    case MethodStart:
        if (!IsChar(input) || IsCtl(input) || IsSpecialChar(input))
        {
            return Failed;
        }
        else
        {
            state_ = Method;
            request.method.push_back(input);
            return Indeterminate;
        }
    case Method:
        if (input == ' ')
        {
            state_ = Uri;
            return Indeterminate;
        }
        else if (!IsChar(input) || IsCtl(input) || IsSpecialChar(input))
        {
            return Failed;
        }
        else
        {
            request.method.push_back(input);
            return Indeterminate;
        }
    case Uri:
        if (input == ' ')
        {
            state_ = HttpVersionH;
            return Indeterminate;
        }
        else if (IsCtl(input))
        {
            return Failed;
        }
        else
        {
            request.uri.push_back(input);
            return Indeterminate;
        }
    case HttpVersionH:
        if (input == 'H')
        {
            state_ = HttpVersionT1;
            return Indeterminate;
        }
        else
        {
            return Failed;
        }
    case HttpVersionT1:
        if (input == 'T')
        {
            state_ = HttpVersionT2;
            return Indeterminate;
        }
        else
        {
            return Failed;
        }
    case HttpVersionT2:
        if (input == 'T')
        {
            state_ = HttpVersionP;
            return Indeterminate;
        }
        else
        {
            return Failed;
        }
    case HttpVersionP:
        if (input == 'P')
        {
            state_ = HttpVersionSlash;
            return Indeterminate;
        }
        else
        {
            return Failed;
        }
    case HttpVersionSlash:
        if (input == '/')
        {
            request.httpVersionMajor = 0;
            request.httpVersionMinor = 0;
            state_ = HttpVersionMajorStart;
            return Indeterminate;
        }
        else
        {
            return Failed;
        }
    case HttpVersionMajorStart:
        if (IsDigit(input))
        {
            request.httpVersionMajor = request.httpVersionMajor * 10 + input - '0';
            state_ = HttpVersionMajor;
            return Indeterminate;
        }
        else
        {
            return Failed;
        }
    case HttpVersionMajor:
        if (input == '.')
        {
            state_ = HttpVersionMinorStart;
            return Indeterminate;
        }
        else if (IsDigit(input))
        {
            request.httpVersionMajor = request.httpVersionMajor * 10 + input - '0';
            return Indeterminate;
        }
        else
        {
            return Failed;
        }
    case HttpVersionMinorStart:
        if (IsDigit(input))
        {
            request.httpVersionMinor = request.httpVersionMinor * 10 + input - '0';
            state_ = HttpVersionMinor;
            return Indeterminate;
        }
        else
        {
            return Failed;
        }
    case HttpVersionMinor:
        if (input == '\r')
        {
            state_ = ExpectingNewline1;
            return Indeterminate;
        }
        else if (IsDigit(input))
        {
            request.httpVersionMinor = request.httpVersionMinor * 10 + input - '0';
            return Indeterminate;
        }
        else
        {
            return Failed;
        }
    case ExpectingNewline1:
        if (input == '\n')
        {
            state_ = HeaderLineStart;
            return Indeterminate;
        }
        else
        {
            return Failed;
        }
    case HeaderLineStart:
        if (input == '\r')
        {
            state_ = ExpectingNewline3;
            return Indeterminate;
        }
        else if (!request.headers.empty() && (input == ' ' || input == '\t'))
        {
            state_ = HeaderLws;
            return Indeterminate;
        }
        else if (!IsChar(input) || IsCtl(input) || IsSpecialChar(input))
        {
            return Failed;
        }
        else
        {
            request.headers.push_back(Header());
            request.headers.back().name.push_back(input);
            state_ = HeaderName;
            return Indeterminate;
        }
    case HeaderLws:
        if (input == '\r')
        {
            state_ = ExpectingNewline2;
            return Indeterminate;
        }
        else if (input == ' ' || input == '\t')
        {
            return Indeterminate;
        }
        else if (IsCtl(input))
        {
            return Failed;
        }
        else
        {
            state_ = HeaderValue;
            request.headers.back().value.push_back(input);
            return Indeterminate;
        }
    case HeaderName:
        if (input == ':')
        {
            state_ = SpaceBeforeHeaderValue;
            return Indeterminate;
        }
        else if (!IsChar(input) || IsCtl(input) || IsSpecialChar(input))
        {
            return Failed;
        }
        else
        {
            request.headers.back().name.push_back(input);
            return Indeterminate;
        }
    case SpaceBeforeHeaderValue:
        if (input == ' ')
        {
            state_ = HeaderValue;
            return Indeterminate;
        }
        else
        {
            return Failed;
        }
    case HeaderValue:
        if (input == '\r')
        {
            state_ = ExpectingNewline2;
            return Indeterminate;
        }
        else if (IsCtl(input))
        {
            return Failed;
        }
        else
        {
            request.headers.back().value.push_back(input);
            return Indeterminate;
        }
    case ExpectingNewline2:
        if (input == '\n')
        {
            state_ = HeaderLineStart;
            return Indeterminate;
        }
        else
        {
            return Failed;
        }
    case ExpectingNewline3:
        return (input == '\n') ? Success : Failed;
    }
    return Failed;
}

} // namespace http

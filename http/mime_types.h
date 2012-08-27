#ifndef HTTP_SERVER3_MIME_TYPES_HPP
#define HTTP_SERVER3_MIME_TYPES_HPP

#include <string>

namespace Http
{
namespace MimeTypes
{

/// Convert a file extension into a MIME type.
std::string ExtensionToType(const std::string& extension);

} // namespace mime_types
} // namespace http

#endif // HTTP_SERVER3_MIME_TYPES_HPP

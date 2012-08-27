#include "mime_types.h"

namespace Http
{
namespace MimeTypes
{

struct mapping
{
    const char* extension;
    const char* mime_type;
};

mapping mappings[] =
{
{ "gif", "image/gif" },
{ "htm", "text/html" },
{ "html", "text/html" },
{ "jpg", "image/jpeg" },
{ "png", "image/png" },
{ 0, 0 } // Marks end of list.
};

std::string ExtensionToType(const std::string& extension)
{
    for (mapping* m = mappings; m->extension; ++m)
    {
        if (m->extension == extension)
        {
            return m->mime_type;
        }
    }

    return "text/plain";
}

} // namespace mime_types
} // namespace http

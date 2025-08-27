#include <cassert>
#include "http_session.h"
#include <httpserver/detail/http_response.h>

namespace http
{
namespace server
{
HttpResponseWriter::HttpResponseWriter(const std::shared_ptr<HttpSession>& session)
    : session_(session)
{
}

HttpResponseWriter::~HttpResponseWriter()
{
    assert(session_);
}

void HttpResponseWriter::send(HttpResponse&& rsp)
{
    assert(session_);
    return session_->writeResponse(std::move(rsp));
}

HttpResponse::HttpResponse(StatusType status, std::string&& body, std::string&& content_type)
    : force_gzip_(false)
    , force_disable_keep_alive_(false)
    , status_(status)
    , content_type_(std::move(content_type))
    , compression_level_(CompressionLevel::BestSpeed)
    , body_(std::move(body))
    , headers_()
{
}

HttpResponse::~HttpResponse()
{
}

HttpResponse& HttpResponse::forceGzip()
{
    force_gzip_ = true;
    return *this;
}

HttpResponse& HttpResponse::forceDisableKeepAlive()
{
    force_disable_keep_alive_ = true;
    return *this;
}

HttpResponse& HttpResponse::compressionLevel(CompressionLevel level)
{
    compression_level_ = level;
    return *this;
}

HttpResponse& HttpResponse::header(const std::string& name, const std::string& value)
{
    headers_[name] = value;
    return *this;
}
}  // namespace server
}  // namespace http

#include "http_session.h"
#include <httpserver/detail/http_response.h>

namespace http
{
namespace server
{
HttpRequest::HttpRequest(uint64_t session_id, uint64_t request_id)
    : session_id_(session_id)
    , request_id_(request_id)
{
}

HttpRequest::~HttpRequest()
{
}

MethodType HttpRequest::method()
{
    return method_;
}

uint64_t HttpRequest::sessionId()
{
    return session_id_;
}

uint64_t HttpRequest::requestId()
{
    return request_id_;
}

const std::map<std::string, std::string>& HttpRequest::headers()
{
    return headers_;
}

const std::map<std::string, std::string>& HttpRequest::params()
{
    return params_;
}

const std::string& HttpRequest::body()
{
    return body_;
}

const std::list<std::string>& HttpRequest::segments()
{
    return segments_;
}

const std::chrono::time_point<std::chrono::steady_clock>& HttpRequest::startTime()
{
    return request_start_time_;
}
}  // namespace server
}  // namespace http
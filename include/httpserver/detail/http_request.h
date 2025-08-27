/**
 * @brief Http request Define
 * @file http_request.h
 * @copyright Licensed under the Apache License, Version 2.0
 */

#pragma once
#include <list>
#include <map>
#include <string>
#include <chrono>
#include "httpserver/detail/http_types.h"

namespace http
{
namespace server
{
// forward declaration
class HttpSession;

class HttpRequest
{
public:
    HttpRequest(uint64_t session_id, uint64_t request_id);
    ~HttpRequest();

    /**
     * @brief return HTTP request method, no exception thrown
     */
    MethodType method();

    /**
     * @brief return HTTP request session id, no exception thrown
     */
    uint64_t sessionId();

    /**
     * @brief return HTTP request id, no exception thrown
     */
    uint64_t requestId();

    /**
     * @brief return request headers, no exception thrown
     */
    const std::map<std::string, std::string>& headers();

    /**
     * @brief return request parameters, no exception thrown
     */
    const std::map<std::string, std::string>& params();

    /**
     * @brief return request body, no exception thrown
     */
    const std::string& body();

    /**
     * @brief return request start time, no exception thrown
     */
    const std::chrono::time_point<std::chrono::steady_clock>& startTime();

    /**
     * @brief return a list of request ulr path segment in order, no exception thrown
     */
    const std::list<std::string>& segments();

private:
    friend class HttpSession;

    MethodType method_;
    uint64_t session_id_;
    uint64_t request_id_;
    std::string body_;
    std::list<std::string> segments_;
    std::map<std::string, std::string> headers_;
    std::map<std::string, std::string> params_;
    std::chrono::time_point<std::chrono::steady_clock> request_start_time_;
};
}  // namespace server
}  // namespace http
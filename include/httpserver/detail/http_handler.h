/**
 * @brief Http handler interface Define
 * @file http_handler.h
 * @copyright Licensed under the Apache License, Version 2.0
 */

#pragma once
#include "httpserver/detail/http_request.h"
#include "httpserver/detail/http_response.h"

namespace http
{
namespace server
{
/**
 * @brief HTTP APIHandler interface
 */
class APIHandler
{
public:
    virtual ~APIHandler();

    /**
     * @brief handler interface
     * @note handle is work on IO thread, and if it need a long time to process </br>
     * it's better to move the request and response_writer to another thread to handle.
     */
    virtual void handle(HttpRequest&& request, HttpResponseWriter&& response_writer) noexcept = 0;
};
}  // namespace server
}  // namespace http

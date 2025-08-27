/**
 * @brief Http response Define
 * @file http_response.h
 * @copyright Licensed under the Apache License, Version 2.0
 */

#pragma once
#include <string>
#include <map>
#include <memory>
#include "httpserver/detail/http_types.h"

namespace http
{
namespace server
{
// forward declaration class;
class HttpSession;

/**
 * @brief HTTP HttpResponse class
 */
class HttpResponse
{
public:
    HttpResponse(StatusType status, std::string&& body, std::string&& content_type);
    ~HttpResponse();

    /**
     * @brief set http response header
     * @param [in] name: http header name
     * @param [in] value: http header value
     */
    HttpResponse& header(const std::string& name, const std::string& value);

    /**
     * @brief ignore http protocol and force gzip of the http response, default false
     */
    HttpResponse& forceGzip();

    /**
     * @brief force disable keep alive, default determine whether to keep alive based on the http protocol
     */
    HttpResponse& forceDisableKeepAlive();

    /**
     * @brief set content gzip compression level, default is CompressionLevel::BestSpeed
     */
    HttpResponse& compressionLevel(CompressionLevel level);

private:
    friend class HttpSession;

    bool force_gzip_;
    bool force_disable_keep_alive_;
    StatusType status_;
    std::string content_type_;
    CompressionLevel compression_level_;
    std::string body_;
    std::map<std::string, std::string> headers_;
};

/**
 * @brief HTTP HttpResponseWriter class
 */
class HttpResponseWriter
{
public:
    explicit HttpResponseWriter(const std::shared_ptr<HttpSession>& session);
    ~HttpResponseWriter();

    /**
     * @brief send the http response, threadsafe, no exception thrown
     * @param [in] rsp: http response
     */
    void send(HttpResponse&& rsp);

private:
    std::shared_ptr<HttpSession> session_;
};

}  // namespace server
}  // namespace http
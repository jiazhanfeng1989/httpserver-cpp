/**
 * @brief Http Session Define
 * @file http_session.h
 * @copyright Licensed under the Apache License, Version 2.0
 */

#pragma once
#include <chrono>
#include <memory>
#include <atomic>
#include <httpserver/http_server.h>
#include "http_common.h"
#include "http_router.h"
#include "http_statistics_internal.h"

namespace http
{
namespace server
{

class HttpSession : public std::enable_shared_from_this<HttpSession>
{
public:
    explicit HttpSession(tcp::socket&& socket,
                         HttpRouter<APIHandler>& router_,
                         const HttpServerOptions& opts,
                         HttpStatisticsInternal& statistics);
    ~HttpSession();
    void run();
    void writeResponse(HttpResponse&& rsp);
    static std::atomic<std::uint64_t> s_id;  // global session id generator

private:
    void doRead();
    void onRead(beast::error_code ec, std::size_t bytes_transferred);
    void doWrite();
    void onWrite(bool keep_alive, beast::error_code ec, std::size_t bytes_transferred);
    void doClose();
    void processRequest();
    std::string compressData(CompressionLevel compression_level, const std::string& uncompressed_data);

private:
    uint64_t id_;
    uint64_t current_request_id_;
    HttpStatisticsInternal& statistics_;
    const HttpServerOptions& opts_;
    HttpRouter<APIHandler>& router_;
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    beast::http::request<beast::http::dynamic_body> request_;
    beast::http::response<beast::http::dynamic_body> response_;
};

}  // namespace server
}  // namespace http
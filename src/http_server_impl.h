/**
 * @brief Http Server Implement Define
 * @file http_server_impl.h
 * @copyright Licensed under the Apache License, Version 2.0
 */

#pragma once
#include <memory>
#include <thread>
#include <vector>
#include <httpserver/http_server.h>
#include "http_common.h"
#include "http_router.h"
#include "http_statistics_internal.h"

namespace http
{
namespace server
{

class HttpServerImpl : public std::enable_shared_from_this<HttpServerImpl>
{
public:
    explicit HttpServerImpl(HttpServerOptions opts);
    ~HttpServerImpl();

    void run();
    void stop();

    HttpStatistics getHttpStatistics();

    void registerHandler(const std::string& path, APIHandler* handler);

private:
    void doAccept();
    void onAccept(beast::error_code ec, tcp::socket socket);
    void resetAllHttpStatistics();

private:
    HttpStatisticsInternal http_statistics_;
    HttpServerOptions opts_;
    HttpRouter<APIHandler> router_;
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::vector<std::thread> io_thread_pool_;
};

}  // namespace server
}  // namespace http
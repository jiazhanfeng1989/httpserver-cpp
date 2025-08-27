#include <cassert>
#include "http_server_impl.h"
#include <httpserver/detail/http_server.h>
#include <httpserver/detail/http_handler.h>

namespace http
{
namespace server
{
APIHandler::~APIHandler()
{
}

HttpServer::HttpServer(HttpServerOptions opts)
    : server_impl_(std::make_shared<HttpServerImpl>(std::move(opts)))
{
}

HttpServer::~HttpServer()
{
}

void HttpServer::run()
{
    assert(server_impl_);
    return server_impl_->run();
}

void HttpServer::stop()
{
    assert(server_impl_);
    return server_impl_->stop();
}

void HttpServer::registerHandler(const std::string& path, APIHandler* handler)
{
    assert(server_impl_);
    return server_impl_->registerHandler(path, handler);
}

HttpStatistics HttpServer::getHttpStatistics()
{
    assert(server_impl_);
    return server_impl_->getHttpStatistics();
}
}  // namespace server
}  // namespace http
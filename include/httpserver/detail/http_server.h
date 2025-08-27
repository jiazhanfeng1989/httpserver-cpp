/**
 * @brief Http Server Define
 * @file http_server.h
 * @copyright Licensed under the Apache License, Version 2.0
 */

#pragma once
#include <string>
#include <memory>
#include <httpserver/detail/http_handler.h>
#include <httpserver/detail/http_types.h>

namespace http
{
namespace server
{
// forward declaration
class HttpServerImpl;

/**
 * @brief HTTP server class
 */
class HttpServer final
{
public:
    explicit HttpServer(HttpServerOptions opts = HttpServerOptions());
    ~HttpServer();

    /**
     * @brief run the http server, not threadsafe, this method will block until stop() is called
     * @throw std::exception if any error occurred
     * @note never call run() repeatedly unless the server is stopped
     */
    void run();

    /**
     * @brief stop the http server, threadsafe
     * @throw std::exception if any error occurred
     */
    void stop();

    /**
     * @brief get http statistics, threadsafe, no exception thrown
     * @return HttpStatistics
     */
    HttpStatistics getHttpStatistics();

    /**
     * @brief register api handler, not threadsafe, should be called before run() function
     * @param [in] path: http uri path
     * @param [in] handler: http request handler
     * @throw std::exception if path is invalid
     * @note  path should start with "/" and not exist relative path such as "../../test",<br>
     * The longest matching algorithm is used for path search, and same path handler will be overwrite.<br>
     * The '/' at the end of path will be ignored, so the "/test/" and "/test" will be treat as same path<br>
     * http server doesn't hold handler life cycle, user should keep handler alive until the server stop.
     */

    void registerHandler(const std::string& path, APIHandler* handler);

private:
    std::shared_ptr<HttpServerImpl> server_impl_;
};

}  // namespace server
}  // namespace http
#include <httpserver/http_server.h>
#include <sstream>
#include <string>
#include "httpserver/detail/http_types.h"
using namespace http::server;

class HelloHandler : public APIHandler
{
public:
    HelloHandler() = default;
    virtual ~HelloHandler() = default;

    virtual void handle(HttpRequest&& request, HttpResponseWriter&& response_writer) noexcept
    {
        auto method = request.method();

        std::stringstream ss;

        // request method
        ss << "receive method: " << static_cast<int>(request.method()) << std::endl;

        // request body
        ss << "receive body: " << request.body() << std::endl;

        // request parms
        for (const auto& p : request.params())
        {
            ss << "receive param: " << p.first << ":" << p.second << std::endl;
        }

        // request headers
        for (const auto& h : request.headers())
        {
            ss << "receive header: " << h.first << ":" << h.second << std::endl;
        }

        // create response
        auto status = StatusType::OK;
        auto body = ss.str();
        const auto* content_type = "text/plain";
        auto rsp = HttpResponse(status, std::move(body), content_type);

        // write response
        response_writer.send(std::move(rsp));
    }
};
int main()
{
    setLogLevel(LogLevel::Info);

    auto server_opts = HttpServerOptions();
    server_opts.thread_num_ = 8;

    auto server = HttpServer(server_opts);
    server.registerHandler("/hello", new HelloHandler());
    server.run();
}
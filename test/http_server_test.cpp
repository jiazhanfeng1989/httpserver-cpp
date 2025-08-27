#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <httpserver/http_server.h>
#include <chrono>
#include <cstddef>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include "http_router.h"
#include "httpserver/detail/http_log.h"

using namespace http::server;
class TestAsyncHandler;
TestAsyncHandler* g_async_handler;
std::shared_ptr<HttpServer> g_server;
std::shared_ptr<std::thread> g_work_thread;

class TestHandler : public APIHandler
{
public:
    TestHandler() = default;
    virtual ~TestHandler() = default;

    virtual void handle(HttpRequest&& request, HttpResponseWriter&& response_writer) noexcept
    {
        auto method = request.method();
        if (request.method() == MethodType::POST)
        {
            std::string body = request.body() + "_rsp";
            std::string content_type = "text/plain";
            response_writer.send(HttpResponse(StatusType::OK, std::move(body), std::move(content_type)));
        }
        else if (request.method() == MethodType::GET)
        {
            const auto& params = request.params();
            CHECK(params.size() > 0);

            const auto& iter = params.find("param");
            CHECK(iter != params.end());
            CHECK(iter->second == "data");
            std::string body = iter->second + "_rsp";
            std::string content_type = "text/plain";
            response_writer.send(HttpResponse(StatusType::OK, std::move(body), std::move(content_type)));
        }
    }
};

class TestTimeoutHandler : public APIHandler
{
public:
    TestTimeoutHandler() = default;
    virtual ~TestTimeoutHandler() = default;

    virtual void handle(HttpRequest&& request, HttpResponseWriter&& response_writer) noexcept
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::string body = "timeout";
        std::string content_type = "text/plain";
        response_writer.send(HttpResponse(StatusType::OK, std::move(body), std::move(content_type)));
    }
};

class TestGzipHandler : public APIHandler
{
public:
    TestGzipHandler() = default;
    virtual ~TestGzipHandler() = default;

    virtual void handle(HttpRequest&& request, HttpResponseWriter&& response_writer) noexcept
    {
        std::string body(1024 * 1024, 'A');
        std::string content_type = "text/plain";
        response_writer.send(HttpResponse(StatusType::OK, std::move(body), std::move(content_type)));
    }
};

void check(const std::map<std::string, std::string> params, const std::string& key, const ::std::string& value)
{
    auto iter = params.find(key);
    CHECK_MESSAGE(iter != params.end(), fmt::format("key {} not found in params", key));
    if (iter != params.end())
    {
        CHECK_MESSAGE(iter->second == value,
                      fmt::format("key {} value mismatch, expect: {}, actual: {}", key, value, iter->second));
    }
}

class TestURLDecodeHandler : public APIHandler
{
public:
    TestURLDecodeHandler() = default;
    virtual ~TestURLDecodeHandler() = default;

    virtual void handle(HttpRequest&& request, HttpResponseWriter&& response_writer) noexcept
    {
        // request parms
        const auto& params = request.params();
        check(params, "output", "default,breakdown,4wd");
        check(params, "data_model_version", "v2");
        check(params, "locale", "eng");
        check(params, "vehicle_profile", "450,180,150,1500");
        check(params, "enable_safety", "true");
        check(params, "live_traffic", "true");
        check(params, "user_id", "b6d2408849c40b23");
        check(params, "destination", "25.670050,-80.376650,0|0,,SW%20113th%20Pl,10639,,,,Unknown%2C25.670170%2C-80.376660%00");
        check(params, "seasonal_restriction", "true");
        check(params, "historical_speed", "true");
        check(params, "speed_in_mps", "10");
        check(params, "request_uuid", "c452bb7b077574a2e47b3d9a5a641f8c-2721f95136ee825aa3b2b0f66a6917e4ac4a1a2c");
        check(params, "start_time", "20240517T032609Z");
        check(params, "heading", "359");
        check(params, "avoid", "permit_required_road");
        check(params, "origin", "25.786223,-80.192353,573663466342783608|159819988380161644,,,,,,,");

        // create response
        auto status = StatusType::OK;
        auto content_type = "text/plain";
        auto rsp = HttpResponse(status, "", std::move(content_type));

        // write response
        response_writer.send(std::move(rsp));
    }
};

class TestSpecialUrlDecodeHandler : public APIHandler
{
public:
    TestSpecialUrlDecodeHandler() = default;
    virtual ~TestSpecialUrlDecodeHandler() = default;

    virtual void handle(HttpRequest&& request, HttpResponseWriter&& response_writer) noexcept
    {
        const auto& params = request.params();

        check(params, "content_level", "Full");
        check(params, "data_model_version", "v2");
        check(params, "destination", "41.921100,12.506910");
        check(params, "origin", "44.400285,11.261346,3044223811510801789|2612104706497283692,,,,,,,");
        check(params, "start_time", "20250425T084337Z");
        check(params, "speed_in_mps", "17");
        check(params, "historical_speed", "true");
        // create response
        auto status = StatusType::OK;
        auto content_type = "text/plain";
        auto rsp = HttpResponse(status, "", std::move(content_type));

        // write response
        response_writer.send(std::move(rsp));
    }
};

class TestAsyncHandler : public APIHandler
{
public:
    using RequestPair = std::pair<HttpRequest, HttpResponseWriter>;
    using RequestQueue = std::list<RequestPair>;

    TestAsyncHandler() = default;
    virtual ~TestAsyncHandler() = default;

    virtual void handle(HttpRequest&& request, HttpResponseWriter&& response_writer) noexcept
    {
        incoming_.emplace_back(std::make_pair(request, response_writer));
        return;
    }

    void run()
    {
        auto func = [this]()
        {
            while (!this->stop_flag_)
            {
                if (incoming_.size() != 0)
                {
                    auto rq = this->incoming_.front();
                    this->incoming_.pop_front();

                    if (rq.first.method() == MethodType::POST)
                    {
                        std::string body = rq.first.body() + "_rsp";
                        std::string content_type = "text/plain";
                        rq.second.send(HttpResponse(StatusType::OK, std::move(body), std::move(content_type)));
                    }
                    else if (rq.first.method() == MethodType::GET)
                    {
                        const auto& params = rq.first.params();
                        CHECK(params.size() > 0);

                        const auto& iter = params.find("param");
                        CHECK(iter != params.end());
                        CHECK(iter->second == "data");
                        std::string body = iter->second + "_rsp";
                        std::string content_type = "text/plain";
                        rq.second.send(HttpResponse(StatusType::OK, std::move(body), std::move(content_type)));
                    }
                }
                else
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
            }
        };
        t_ = std::move(std::thread(func));
    }
    void stop()
    {
        stop_flag_.store(true);
        t_.join();
    }

private:
    std::thread t_;
    std::atomic_bool stop_flag_{false};
    RequestQueue incoming_;
};

// Global test setup and teardown functions
static void setupTestSuite()
{
    setLogLevel(LogLevel::Info);
    g_work_thread = std::make_shared<std::thread>(std::thread(
        []()
        {
            auto opts = HttpServerOptions();
            opts.read_time_out_ = 1;
            opts.write_time_out_ = 1;
            g_async_handler = new TestAsyncHandler();
            g_server = std::make_shared<HttpServer>(opts);
            g_server->registerHandler("/hello", new TestHandler());
            g_server->registerHandler("/timeout", new TestTimeoutHandler());
            g_server->registerHandler("/gzip", new TestGzipHandler());
            g_server->registerHandler("/url", new TestURLDecodeHandler());
            g_server->registerHandler("/async", g_async_handler);
            g_async_handler->run();
            g_server->run();
        }));
}

static void teardownTestSuite()
{
    g_server->stop();
    g_async_handler->stop();
    g_work_thread->join();
}

TEST_SUITE_BEGIN("HttpServer");

TEST_CASE("TestHttpLog")
{
    // default is console mode
    setLogLevel(LogLevel::Trace);
    LOG_LOGGER_TRACE("console trace message");
    LOG_LOGGER_DEBUG("console debug message");
    LOG_LOGGER_INFO("console info message");
    LOG_LOGGER_WARN("console warning message");
    LOG_LOGGER_ERROR("console error message");
    LOG_LOGGER_CRITICAL("console critical message");

    // disable console mode, and enable file mode
    auto opts = LogOptions();
    opts.enable_file_mode_ = true;  // enable file mode
    opts.file_mode_options_.file_name_ = "http_server_test_log.txt";
    opts.file_mode_options_.truncate_ = true;
    opts.enable_console_mode_ = true;  // enable console mode, logs will be written to files and console

    // init logger with options
    initLog(opts);
    setLogLevel(LogLevel::Info);

    LOG_LOGGER_TRACE("file trace message");
    LOG_LOGGER_DEBUG("file debug message");
    LOG_LOGGER_INFO("file info message");
    LOG_LOGGER_WARN("file warning message");
    LOG_LOGGER_ERROR("file error message");
    LOG_LOGGER_CRITICAL("file critical message");

    // reinit will throw exception
    CHECK_THROWS_AS(initLog(opts), std::runtime_error);
}

TEST_CASE("TestHttpRouter")
{
    using DataType = std::string;
    HttpRouter<DataType> path_tree;
    CHECK_NOTHROW(path_tree.insert("/", new DataType("data_root")));
    CHECK_NOTHROW(path_tree.insert("/hello", new DataType("data_hello")));
    CHECK_NOTHROW(path_tree.insert("/hello/test", new DataType("data_hello_test")));
    CHECK_NOTHROW(path_tree.insert("/hello/test/abc/", new DataType("data_hello_test_abc")));
    CHECK_THROWS_AS(path_tree.insert("/..", new DataType("data_invalid")), std::runtime_error);
    CHECK_THROWS_AS(path_tree.insert("/../abc", new DataType("data_invalid")), std::runtime_error);
    CHECK_THROWS_AS(path_tree.insert("abc", new DataType("data_invalid")), std::runtime_error);

    CHECK(!path_tree.search("/abc"));                     // not match
    CHECK(!path_tree.search("//"));                       // not match
    CHECK(!path_tree.search("/he"));                      // not match
    CHECK(!path_tree.search("hello"));                    // path should start with '/', not match
    CHECK(*path_tree.search("") == "data_root");          // math register path '/'
    CHECK(*path_tree.search("/") == "data_root");         // math register path '/'
    CHECK(*path_tree.search("/hello") == "data_hello");   // math register path '/hello'
    CHECK(*path_tree.search("/hello/") == "data_hello");  // math register path '/hello'
    CHECK(*path_tree.search("/hello/test") == "data_hello_test");  // math register path '/hello/test'
    CHECK(*path_tree.search("/hello/test/") == "data_hello_test");  // math register  path '/hello/test'
    CHECK(*path_tree.search("/hello/abc/def") == "data_hello");  // math register path '/hello'
    CHECK(*path_tree.search("/hello/test/abc/") == "data_hello_test_abc");  // math register path '/hello/test/abc/'
    CHECK(*path_tree.search("/hello/test/abc") == "data_hello_test_abc");  // math register path '/hello/test/abc/'
}

#include <spdlog/fmt/bundled/core.h>
#include <stdexcept>
#include "httpserver/detail/http_log.h"
#include "http_server_impl.h"
#include "http_session.h"

namespace http
{
namespace server
{

HttpServerImpl::HttpServerImpl(HttpServerOptions opts)
    : opts_(std::move(opts))
    , router_()
    , io_context_(opts_.thread_num_)
    , acceptor_(boost::asio::make_strand(io_context_))
    , io_thread_pool_()
{
    io_context_.stop();
}

HttpServerImpl::~HttpServerImpl()
{
    LOG_LOGGER_INFO("HttpServerImpl destroyed");
}

void HttpServerImpl::run()
{
    if (opts_.thread_num_ == 0)
    {
        throw std::runtime_error("thread count should > 0");
    }

    if (opts_.max_request_size_ == 0)
    {
        throw std::runtime_error("max request should > 0");
    }

    if (opts_.addr_.empty())
    {
        throw std::runtime_error("addr is empty");
    }

    HttpSession::s_id.store(0);  // reset global session id
    resetAllHttpStatistics();    // reset all http statics

    io_context_.restart();
    auto endpoint = tcp::endpoint{net::ip::make_address(opts_.addr_), opts_.port_};
    beast::error_code ec;
    acceptor_.open(endpoint.protocol(), ec);
    if (ec)
    {
        throw std::runtime_error(ec.message());
    }

    // address reuse
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec)
    {
        throw std::runtime_error(ec.message());
    }

    acceptor_.set_option(net::socket_base::linger(false, 1), ec);
    if (ec)
    {
        throw std::runtime_error(ec.message());
    }

    // bind to the server address
    acceptor_.bind(endpoint, ec);
    if (ec)
    {
        throw std::runtime_error(ec.message());
    }

    // start listening for connections
    acceptor_.listen(1024, ec);
    if (ec)
    {
        throw std::runtime_error(ec.message());
    }

    LOG_LOGGER_INFO(fmt::format("start listen on: {}, thread_num: {}, read_time_out: {}s, write_time_out: {}s, auto_gzip: {}, max_request_size: {}KB auto_decode_url_parameters: {}",
                                endpoint.address().to_string() + ":" + std::to_string(endpoint.port()),
                                opts_.thread_num_,
                                opts_.read_time_out_,
                                opts_.write_time_out_,
                                opts_.auto_gzip_,
                                opts_.max_request_size_ / 1024,
                                opts_.auto_decode_url_parameters_));

    doAccept();

    for (auto i = 0; i < opts_.thread_num_ - 1; ++i)
    {
        io_thread_pool_.emplace_back([this] { this->io_context_.run(); });
    }

    io_context_.run();
}

void HttpServerImpl::stop()
{
    LOG_LOGGER_INFO("HttpServerImpl begin stop");
    if (!io_context_.stopped())
    {
        io_context_.stop();

        for (std::thread& thread : io_thread_pool_)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
        io_thread_pool_.clear();
    }

    if (acceptor_.is_open())
    {
        beast::error_code ec;
        acceptor_.close();
        if (ec)
        {
            throw std::runtime_error(ec.message());
        }
    }

    LOG_LOGGER_INFO("HttpServerImpl end stop");
}

void HttpServerImpl::registerHandler(const std::string& path, APIHandler* handler)
{
    router_.insert(path, handler);
}

void HttpServerImpl::doAccept()
{
    acceptor_.async_accept(net::make_strand(io_context_),
                           beast::bind_front_handler(&HttpServerImpl::onAccept, shared_from_this()));
}

void HttpServerImpl::onAccept(beast::error_code ec, tcp::socket socket)
{
    if (!ec)
    {
        // create the session and run it
        std::make_shared<HttpSession>(std::move(socket), router_, opts_, http_statistics_)->run();
    }

    // accept another connection
    doAccept();
}

void HttpServerImpl::resetAllHttpStatistics()
{
    http_statistics_.session_cnt_.store(0);
    http_statistics_.read_timeout_cnt_.store(0);
    http_statistics_.handle_request_cnt_.store(0);
    http_statistics_.read_fail_cnt_.store(0);
    http_statistics_.read_success_cnt_.store(0);
    http_statistics_.write_timeout_cnt_.store(0);
    http_statistics_.write_fail_cnt_.store(0);
    http_statistics_.write_success_cnt_.store(0);
    http_statistics_.working_handler_cnt_.store(0);
}

HttpStatistics HttpServerImpl::getHttpStatistics()
{
    HttpStatistics statics;
    statics.handler_request_cnt_ = http_statistics_.handle_request_cnt_.load();
    statics.working_handler_cnt_ = http_statistics_.working_handler_cnt_.load();
    statics.read_timeout_cnt_ = http_statistics_.read_timeout_cnt_.load();
    statics.read_success_cnt_ = http_statistics_.read_success_cnt_.load();
    statics.read_fail_cnt_ = http_statistics_.read_fail_cnt_.load();
    statics.write_timeout_cnt_ = http_statistics_.write_timeout_cnt_.load();
    statics.write_success_cnt_ = http_statistics_.write_success_cnt_.load();
    statics.write_fail_cnt_ = http_statistics_.write_fail_cnt_.load();
    statics.session_cnt_ = http_statistics_.session_cnt_.load();
    return statics;
}

}  // namespace server
}  // namespace http
#include <cassert>
#include "http_session.h"
#include "httpserver/detail/http_types.h"
#include "httpserver/detail/http_log.h"

namespace http
{
namespace server
{
std::atomic<std::uint64_t> HttpSession::s_id{0};

HttpSession::HttpSession(tcp::socket&& socket,
                         HttpRouter<APIHandler>& router,
                         const HttpServerOptions& opts,
                         HttpStatisticsInternal& statistics)
    : id_(++s_id)
    , current_request_id_(0)
    , statistics_(statistics)
    , opts_(opts)
    , router_(router)
    , stream_(std::move(socket))
    , buffer_(opts.max_request_size_)
    , request_()
    , response_()
{
    ++statistics_.session_cnt_;
    beast::error_code ec;
    auto remote_endpoint = stream_.socket().remote_endpoint(ec);
    if (!ec)
    {
        LOG_LOGGER_TRACE(fmt::format("session[{}] create, remote: {}",
                                     id_,
                                     remote_endpoint.address().to_string() + ":" +
                                         std::to_string(remote_endpoint.port())));
    }
}

HttpSession::~HttpSession()
{
    --statistics_.session_cnt_;
    doClose();
    LOG_LOGGER_TRACE(fmt::format("session[{}] destroy", id_));
}

void HttpSession::run()
{
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session.
    net::dispatch(stream_.get_executor(), beast::bind_front_handler(&HttpSession::doRead, shared_from_this()));
}

void HttpSession::doRead()
{
    ++current_request_id_;
    if (opts_.read_time_out_ != 0)
    {
        // set read timeout
        stream_.expires_after(std::chrono::seconds(opts_.read_time_out_));
    }
    else
    {
        // never timeout
        stream_.expires_never();
    }

    // read a request
    beast::http::async_read(stream_,
                            buffer_,
                            request_,
                            beast::bind_front_handler(&HttpSession::onRead, shared_from_this()));
}

void HttpSession::onRead(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);
    if (ec == beast::http::error::end_of_stream)
    {
        // remote exit, normal close
        return doClose();
    }
    else if (ec == beast::error::timeout)
    {
        ++statistics_.read_timeout_cnt_;
        LOG_LOGGER_TRACE(fmt::format("close invalid session[{}], request_id: {}, read fail: timeout", id_, current_request_id_));
        return doClose();
    }
    else if (ec)
    {
        ++statistics_.read_fail_cnt_;
        LOG_LOGGER_TRACE(fmt::format("close invalid session[{}], request_id: {}, read fail: {}",
                                     id_,
                                     current_request_id_,
                                     ec.message()));
        return doClose();
    }
    else
    {
        LOG_LOGGER_TRACE(fmt::format("session[{}] request_id: {}, read success", id_, current_request_id_));
        ++statistics_.read_success_cnt_;
        processRequest();
    }
}

void HttpSession::doWrite()
{
    if (opts_.write_time_out_ != 0)
    {
        // set write timeout
        stream_.expires_after(std::chrono::seconds(opts_.write_time_out_));
    }
    else
    {
        // never timeout
        stream_.expires_never();
    }

    beast::http::async_write(stream_,
                             response_,
                             beast::bind_front_handler(&HttpSession::onWrite,
                                                       shared_from_this(),
                                                       response_.keep_alive()));
}

void HttpSession::onWrite(bool keep_alive, beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec == beast::error::timeout)
    {
        ++statistics_.write_timeout_cnt_;
        LOG_LOGGER_ERROR(fmt::format("close invalid session[{}], request_id: {}, write fail: timeout", id_, current_request_id_));
        return doClose();
    }
    else if (ec)
    {
        ++statistics_.write_fail_cnt_;
        LOG_LOGGER_ERROR(fmt::format("close invalid session[{}], request_id: {}, write fail: {}",
                                     id_,
                                     current_request_id_,
                                     ec.message()));
        return doClose();
    }

    if (!keep_alive)
    {
        // this means we should close the connection, usually because
        // the response indicated the "Connection: close".
        ++statistics_.write_success_cnt_;
        LOG_LOGGER_TRACE(fmt::format("session[{}] request_id: {}, keep alive is false, should close", id_, current_request_id_));
        return doClose();
    }

    response_.clear();
    response_.body().clear();
    ++statistics_.write_success_cnt_;
    LOG_LOGGER_TRACE(fmt::format("session[{}] request_id: {}, onWrite success", id_, current_request_id_));

    // read another request
    doRead();
}

void HttpSession::doClose()
{
    if (stream_.socket().is_open())
    {
        stream_.close();
        LOG_LOGGER_TRACE(fmt::format("session[{}] closed", id_));
    }
}

void HttpSession::processRequest()
{
    // parse uri
    boost::system::result<boost::url_view> r;
    std::string origin_target;
    if (request_.method() == beast::http::verb::get && request_.target().find('|') != boost::string_view::npos)
    {
        // replace '|' with '%7C' to avoid parse error
        origin_target.assign(request_.target().data(), request_.target().size());
        boost::replace_all(origin_target, "|", "%7C");
        r = urls::parse_origin_form(origin_target);
    }
    else
    {
        r = urls::parse_origin_form(request_.target());
    }

    if (r.has_error())
    {
        ++statistics_.handle_request_cnt_;
        HttpResponse rsp(StatusType::Bad_Request, "url invalid", "text/plain");
        writeResponse(std::move(rsp));
        LOG_LOGGER_ERROR(fmt::format("session[{}], request_id: {}, parse url fail: {}",
                                     id_,
                                     current_request_id_,
                                     r.error().message()));
        request_.body().clear();
        return;
    }

    auto url = r.value();
    auto segments = url.segments();
    auto handler = router_.search(segments);
    if (handler == nullptr)
    {
        // handler not found
        LOG_LOGGER_ERROR(fmt::format("session[{}], request_id: {}, handler not found", id_, current_request_id_));
        ++statistics_.handle_request_cnt_;
        HttpResponse rsp(StatusType::Bad_Request, "current url not support", "text/plain");
        writeResponse(std::move(rsp));
        request_.body().clear();
        return;
    }

    // create http request
    auto request = HttpRequest(id_, current_request_id_);
    request.request_start_time_ = std::chrono::steady_clock::now();

    // set http method
    if (request_.method() > beast::http::verb::put)
    {
        // handler not found
        LOG_LOGGER_ERROR(fmt::format("session[{}], request_id: {}, method not support", id_, current_request_id_));
        ++statistics_.handle_request_cnt_;
        HttpResponse rsp(StatusType::Bad_Request, "current method not support", "text/plain");
        writeResponse(std::move(rsp));
        request_.body().clear();
        return;
    }

    request.method_ = static_cast<MethodType>(request_.method());

    // set http header
    for (auto iter = request_.begin(); iter != request_.end(); iter++)
    {
        request.headers_[iter->name_string()] = iter->value();
    }

    // set http path segments
    for (auto s : segments)
    {
        request.segments_.push_back(s);
    }

    // set http parameters
    if (opts_.auto_decode_url_parameters_)
    {
        for (auto p : url.params())
        {
            request.params_[p.key] = p.value;
        }
    }
    else
    {
        for (auto p : url.encoded_params())
        {
            request.params_[std::string(p.key)] = std::string(p.value);
        }
    }

    // set http body
    if (request_.body().size() > 0)
    {
        auto& body = request_.body();
        request.body_ = beast::buffers_to_string(request_.body().data());
        request_.body().clear();
    }

    ++statistics_.working_handler_cnt_;
    handler->handle(std::move(request), HttpResponseWriter(shared_from_this()));
    --statistics_.working_handler_cnt_;
    ++statistics_.handle_request_cnt_;
}

void HttpSession::writeResponse(HttpResponse&& rsp)
{
    // common header
    if (rsp.force_disable_keep_alive_)
    {
        response_.keep_alive(false);
    }
    else
    {
        response_.keep_alive(request_.keep_alive());
    }

    response_.result(static_cast<unsigned int>(rsp.status_));
    response_.set(beast::http::field::content_type, rsp.content_type_);

    // user set header
    for (auto& p : rsp.headers_)
    {
        response_.set(p.first, p.second);
    }

    if (rsp.force_gzip_)
    {
        // force gzip
        response_.set(beast::http::field::content_encoding, "gzip");
        rsp.body_ = compressData(rsp.compression_level_, rsp.body_);
    }
    else
    {
        // body > 500 bytes, protocol gzip
        if (rsp.body_.size() > 500 && opts_.auto_gzip_ &&
            (boost::icontains(request_[beast::http::field::accept_encoding], "gzip") ||
             boost::icontains(request_[beast::http::field::accept_encoding], "*")))
        {
            response_.set(beast::http::field::content_encoding, "gzip");
            rsp.body_ = compressData(rsp.compression_level_, rsp.body_);
        }
    }

    if (rsp.body_.size() > 0)
    {
        // http header method doesn't require body
        if (request_.method() != beast::http::verb::head)
        {
            beast::ostream(response_.body()) << std::move(rsp.body_);
        }
        response_.prepare_payload();
    }

    doWrite();
}

std::string HttpSession::compressData(CompressionLevel compression_level, const std::string& uncompressed_data)
{
    boost::iostreams::gzip_params compression_parameters;

    // Set the compression level based on the provided parameter
    compression_parameters.level = static_cast<int>(compression_level);

    std::string compressed_data;
    boost::iostreams::filtering_ostream gzip_stream;
    gzip_stream.push(boost::iostreams::gzip_compressor(compression_parameters));
    gzip_stream.push(boost::iostreams::back_inserter(compressed_data));
    gzip_stream.write(uncompressed_data.data(), uncompressed_data.size());
    boost::iostreams::close(gzip_stream);
    return compressed_data;
}
}  // namespace server
}  // namespace http
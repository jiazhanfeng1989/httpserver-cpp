/**
 * @brief Http Types Define
 * @file http_types.h
 * @copyright Licensed under the Apache License, Version 2.0
 */

#pragma once
#include <string>
#include <cstdint>

namespace http
{
namespace server
{
/**
 * @brief HTTP server options
 */
struct HttpServerOptions
{
    std::string addr_{"0.0.0.0"};  ///< http server ipv4 addr
    uint16_t port_{6000};          ///< http server ipv4 addr port, default 6000
    uint32_t thread_num_{1};       ///< http server work thread number, default 1
    uint64_t read_time_out_{60};  ///< read req timeout, uint:seconds, default 60s, 0 means not timeout
    uint64_t write_time_out_{60};  ///< write rsp timeout, uint:seconds, default 60s, 0 means not timeout
    uint64_t max_request_size_{2097152};  ///< http request max length, if it overflow, will close the connection, default 2MB
    bool auto_gzip_{true};  ///< when the accept_encoding of request is set and auto_gzip_ is true, server automatically gzip the response body
    bool auto_decode_url_parameters_{true};  ///< whether decode url parameters automatically.
};

/**
 * @brief HTTP statistics
 */
struct HttpStatistics
{
    uint32_t session_cnt_{0};  ///< http server current session count
    uint64_t read_timeout_cnt_{0};  ///< http server read time timeout count
    uint64_t read_success_cnt_{0};  ///< http server read request success count
    uint64_t read_fail_cnt_{0};  ///< http server read request fail count, not include timeout fail
    uint64_t write_timeout_cnt_{0};  ///< http server timeout count, include write time and handle request time
    uint64_t write_success_cnt_{0};  ///< http server write response success count
    uint64_t write_fail_cnt_{0};  ///< http server write response fail count, not include timeout fail
    uint64_t handler_request_cnt_{0};  ///< http server handle request count
    uint64_t working_handler_cnt_{0};  ///< http server current is working handler count
};

/**
 * @brief HTTP status type
 */
enum class StatusType
{
    OK = 200,           ///< http 200, the request succeeded.
    Bad_Request = 400,  ///< http 400, the server cannot or will not process the request due to something that is perceived to be a client error
    Internal_Server_Error = 500,  ///< http 500, the server has encountered a situation it does not know how to handle.
    Service_Temporary_Unavailable = 503  /// http 503, the server is temporarily unable to process client requests due to overloading or system maintenance.
};

/**
 * @brief HTTP method
 */
enum class MethodType
{
    Unknown = 0,  ///< http unknown method
    Delete = 1,   ///< http delete method
    GET = 2,      ///< http get method
    HEAD = 3,     ///< http head method
    POST = 4,     ///< http post method
    PUT = 5       ///< http put method
};

/**
 * @brief HTTP Content gzip compression level
 */
enum class CompressionLevel
{
    NoCompression = 0,
    BestSpeed = 1,
    BestCompression = 9,
    DefaultCompression = -1
};
}  // namespace server
}  // namespace http
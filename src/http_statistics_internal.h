/**
 * @brief Http statistics Define
 * @file http_statistics_internal.h
 * @copyright Licensed under the Apache License, Version 2.0
 */

#pragma once
#include <atomic>

namespace http
{
namespace server
{
struct HttpStatisticsInternal
{
    std::atomic<std::uint32_t> session_cnt_{0};
    std::atomic<std::uint64_t> read_timeout_cnt_{0};
    std::atomic<std::uint64_t> read_success_cnt_{0};
    std::atomic<std::uint64_t> read_fail_cnt_{0};
    std::atomic<std::uint64_t> write_timeout_cnt_{0};
    std::atomic<std::uint64_t> write_success_cnt_{0};
    std::atomic<std::uint64_t> write_fail_cnt_{0};
    std::atomic<std::uint64_t> handle_request_cnt_{0};
    std::atomic<std::uint64_t> working_handler_cnt_{0};
};

}  // namespace server
}  // namespace http
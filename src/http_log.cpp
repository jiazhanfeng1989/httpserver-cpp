#include "httpserver/detail/http_log.h"
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <spdlog/common.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace http
{
namespace server
{

const char *default_log_name = "http server";
const char *default_log_pattern = "%Y-%m-%dT%H:%M:%S.%e|%L|%t|%n|%v|%s|line#%#|";

static bool init_flag = false;

void initLog(const LogOptions &opts)
{
    if (init_flag)
    {
        throw std::runtime_error("logger was already initialized, call the function once at program start (or never)");
    }
    init_flag = true;

    std::vector<spdlog::sink_ptr> sinks;
    if (opts.enable_console_mode_)
    {
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    }

    if (opts.enable_file_mode_)
    {
        if (opts.file_mode_options_.file_name_ == "")
        {
            throw std::runtime_error("logger filename is empty when file mode enable");
        }

        if (opts.file_mode_options_.file_size_ == 0 || opts.file_mode_options_.files_count_ == 0)
        {
            // basic file
            sinks.push_back(
                std::make_shared<spdlog::sinks::basic_file_sink_mt>(opts.file_mode_options_.file_name_,
                                                                    opts.file_mode_options_.truncate_));
        }
        else
        {
            // rotating file
            sinks.push_back(
                std::make_shared<spdlog::sinks::rotating_file_sink_mt>(opts.file_mode_options_.file_name_,
                                                                       opts.file_mode_options_.file_size_,
                                                                       opts.file_mode_options_.files_count_));
        }
    }

    if (sinks.size() == 0)
    {
        throw std::runtime_error("there is no valid logger sinks");
    }

    if (opts.enable_async_mode_)
    {
        if (opts.async_mode_options_.queue_size_ == 0 || opts.async_mode_options_.thread_count_ == 0)
        {
            throw std::runtime_error("logger queue size or thread count is invalid");
        }

        spdlog::init_thread_pool(opts.async_mode_options_.queue_size_, opts.async_mode_options_.thread_count_);
        auto logger = std::make_shared<spdlog::async_logger>(default_log_name,
                                                             sinks.begin(),
                                                             sinks.end(),
                                                             spdlog::thread_pool(),
                                                             spdlog::async_overflow_policy::block);

        spdlog::set_default_logger(logger);
        spdlog::set_pattern(default_log_pattern);
    }
    else
    {
        auto logger = std::make_shared<spdlog::logger>(default_log_name, sinks.begin(), sinks.end());
        spdlog::set_default_logger(logger);
        spdlog::set_pattern(default_log_pattern);
    }
}

void setLogLevel(LogLevel lvl)
{
    spdlog::set_level(static_cast<spdlog::level::level_enum>(lvl));
}

LogLevel getLogLevel()
{
    return static_cast<LogLevel>(spdlog::get_level());
}

void httpLog(const SourceLoc &loc, LogLevel lvl, const std::string &msg)
{
    static bool init = []
    {
        spdlog::set_pattern(default_log_pattern);
        return true;
    }();

    spdlog::log(spdlog::source_loc(loc.file_name_, loc.line_, loc.func_name_),
                static_cast<spdlog::level::level_enum>(lvl),
                msg);
}

}  // namespace server
}  // namespace http
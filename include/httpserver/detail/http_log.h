/**
 * @brief Http log Define
 * @file http_log.h
 * @copyright Licensed under the Apache License, Version 2.0
 */
#pragma once
#include <cassert>
#include <memory>
#include <string>

namespace http
{
namespace server
{

enum class LogLevel
{
    Trace = 0,     ///< trace log level
    Debug = 1,     ///< debug log level
    Info = 2,      ///< info log level
    Warn = 3,      ///< warn log level
    Err = 4,       ///< error log level
    Critical = 5,  ///< critical log level
    Off = 6,       ///< off all log output
    Invalid,
};

/**
 * @brief log async mode options
 */
struct AsyncModeOptions
{
    uint64_t queue_size_{10000};  ///< logger queue items size
    uint64_t thread_count_{1};    ///< logger work thread count
};

/**
 * @brief log file mode options
 */
struct FileModeOptions
{
    std::string file_name_{""};  ///< log file name
    uint64_t file_size_{0};      ///< rotating logger with the file size, 0 means disable rotating
    uint64_t files_count_{0};    ///< rotating max files count, 0 means disable rotating
    bool truncate_{false};       ///< truncate when file exist, works on rotating disable
};

/**
 * @brief log options
 * @note async mode is optional for performance using.<br>
 *       if console mode enable, logs are written to console.<br>
 *       if file mode enable, logs are written to files.<br>
 *       if console mode and file mode both enable, logs are written to console and files.
 *       default console mode is enable.
 */
struct LogOptions
{
    bool enable_console_mode_{true};       ///< logs are also written to console
    bool enable_file_mode_{false};         ///< logger enable file mode.
    bool enable_async_mode_{false};        ///< logger enable async mode.
    FileModeOptions file_mode_options_;    ///< only works on enable_file_mode is true;
    AsyncModeOptions async_mode_options_;  ///< only works on enable_async_mode is true;
};

/**
 * @brief source location defintion
 */
struct SourceLoc
{
    SourceLoc() = default;
    SourceLoc(const char *file_name, int line, const char *func_name)
        : file_name_(file_name)
        , line_(line)
        , func_name_(func_name)
    {
    }

    const char *file_name_{nullptr};
    int line_{0};
    const char *func_name_{nullptr};
};

/**
 * @brief global log initializer
 * @note In most cases you don't need call this otherwise you want change log mode<br>
 * keep in mind that this function is not thread safe and is intended to be called exactly<br>
 * once at program start (or never), default is console mode, logs are written to console.
 * @throw std::exception if opts is invalid
 */
void initLog(const LogOptions &opts);

/**
 * @brief set global log level, threadsafe, default is LogLevel::Info
 */
void setLogLevel(LogLevel lvl);

/**
 * @brief get global log level, threadsafe
 */
LogLevel getLogLevel();

/**
 * @brief log function defintion
 */
void httpLog(const SourceLoc &loc, LogLevel lvl, const std::string &msg);

/**
 * @brief easy to use logger helper macro definition, it's recommend to use the these macros.
 */
#define LOG_LOGGER_TRACE(msg)                                                                         \
    if (getLogLevel() <= http::server::LogLevel::Trace)                                               \
    {                                                                                                 \
        httpLog(http::server::SourceLoc(__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)), \
                http::server::LogLevel::Trace,                                                        \
                msg);                                                                                 \
    }

#define LOG_LOGGER_DEBUG(msg)                                                                         \
    if (getLogLevel() <= http::server::LogLevel::Debug)                                               \
    {                                                                                                 \
        httpLog(http::server::SourceLoc(__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)), \
                http::server::LogLevel::Debug,                                                        \
                msg);                                                                                 \
    }

#define LOG_LOGGER_INFO(msg)                                                                          \
    if (getLogLevel() <= http::server::LogLevel::Info)                                                \
    {                                                                                                 \
        httpLog(http::server::SourceLoc(__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)), \
                http::server::LogLevel::Info,                                                         \
                msg);                                                                                 \
    }

#define LOG_LOGGER_WARN(msg)                                                                          \
    if (getLogLevel() <= http::server::LogLevel::Warn)                                                \
    {                                                                                                 \
        httpLog(http::server::SourceLoc(__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)), \
                http::server::LogLevel::Warn,                                                         \
                msg);                                                                                 \
    }

#define LOG_LOGGER_ERROR(msg)                                                                         \
    if (getLogLevel() <= http::server::LogLevel::Err)                                                 \
    {                                                                                                 \
        httpLog(http::server::SourceLoc(__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)), \
                http::server::LogLevel::Err,                                                          \
                msg);                                                                                 \
    }

#define LOG_LOGGER_CRITICAL(msg)                                                                      \
    if (getLogLevel() <= http::server::LogLevel::Critical)                                            \
    {                                                                                                 \
        httpLog(http::server::SourceLoc(__FILE__, __LINE__, static_cast<const char *>(__FUNCTION__)), \
                http::server::LogLevel::Critical,                                                     \
                msg);                                                                                 \
    }
}  // namespace server
}  // namespace http
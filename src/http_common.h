/**
 * @brief Http Common Define
 * @file http_common_.h
 * @copyright Licensed under the Apache License, Version 2.0
 */

#pragma once
#include <boost/url.hpp>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/string.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/beast/http/verb.hpp>
#include <spdlog/fmt/bundled/core.h>
#include <boost/system/result.hpp>

namespace beast = boost::beast;  // from <boost/beast.hpp>
namespace net = boost::asio;     // from <boost/asio.hpp>
namespace urls = boost::urls;    // from <boost/url.hpp>

using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

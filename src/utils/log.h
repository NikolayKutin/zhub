// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>

enum class log_severity_level { debug, info, warning, error, fatal };

template< typename CharT, typename TraitsT > inline std::basic_ostream< CharT, TraitsT >& operator<< (std::basic_ostream< CharT, TraitsT >& strm, log_severity_level severity) {
    switch (severity) {
    case log_severity_level::debug:      strm << "DEBUG";    break;
    case log_severity_level::info:       strm << "INFO";     break;
    case log_severity_level::warning:    strm << "WARNING";  break;
    case log_severity_level::error:      strm << "ERROR";    break;
    case log_severity_level::fatal:      strm << "FATAL";    break;
    }
 
    return strm;
}

BOOST_LOG_GLOBAL_LOGGER(ups_logger, boost::log::sources::severity_logger_mt<log_severity_level>)

#define LOG_DEBUG       BOOST_LOG_SEV(ups_logger::get(), log_severity_level::debug)
#define LOG_INFO        BOOST_LOG_SEV(ups_logger::get(), log_severity_level::info)
#define LOG_WARNING     BOOST_LOG_SEV(ups_logger::get(), log_severity_level::warning)
#define LOG_ERROR       BOOST_LOG_SEV(ups_logger::get(), log_severity_level::error)
#define LOG_FATAL_ERROR BOOST_LOG_SEV(ups_logger::get(), log_severity_level::fatal)
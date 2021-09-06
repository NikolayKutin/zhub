// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

//_SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING

#include "zhub.h"

void addUnitsSupport();
void addZigbeeDevicesSupport();

BOOST_LOG_GLOBAL_LOGGER_INIT(ups_logger, src::severity_logger_mt<log_severity_level>)
{
    boost::log::sources::severity_logger_mt<log_severity_level> lg;
    boost::log::add_common_attributes();
    boost::log::add_console_log(std::clog, boost::log::keywords::format = boost::log::expressions::format("[%1%] %2%: %3%")
        % boost::log::expressions::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
        % boost::log::expressions::attr<log_severity_level>("Severity")
        % boost::log::expressions::message);
    return lg;
}

int main(int argc, char* argv[])
{
    addZigbeeDevicesSupport();
    addUnitsSupport();

    boost::asio::io_service io_service;
    Zhub zhub(io_service);

    setlocale(LC_ALL, "Russian");

    try {
        zhub.init_from_yaml("configuration.yaml");
    }
    catch (std::runtime_error& error) {
        LOG_FATAL_ERROR << error.what();
        return EXIT_FAILURE;
    }

    boost::asio::io_service::work idle(io_service);
    io_service.run();

    return EXIT_SUCCESS;
}
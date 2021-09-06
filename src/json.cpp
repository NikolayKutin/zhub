// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#include "json.h"

std::string toJson(FunctionalUnit& unit) {
    std::stringstream sstream;
    sstream << "{\"type\": \"" << unit.type() << "\", \"id\": \"" << unit.id() << "\", \"devices\": [";

    auto devices = std::move(unit.devices());
    for (auto it = devices.begin(); it != devices.end();) {
        sstream << zigbee::toJson(**it);

        if (++it != devices.end()) sstream << ", ";
    }
    sstream << "]}";

    return std::move(sstream.str());
}
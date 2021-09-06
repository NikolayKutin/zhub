// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <stdint.h>
#include <memory>
#include <exception>
#include <mutex>
#include <atomic>
#include <cassert>
#include <vector>
#include <map>
#include <chrono>
#include <variant>
#include <any>
#include <optional>
#include <iostream> // TODO: delete
#include <system_error> // TODO: ?

#include <boost/cstdfloat.hpp>
#include <boost/signals2.hpp>
#include <boost/asio.hpp>
#include <boost/format.hpp>

#include "../utils/utils.h"

namespace zigbee {

    #include "zcl.h"
    #include "attribute.h"
    #include "types.h"
    #include "adapter.h"
    #include "controller.h"
    #include "end_device.h"
    #include "coordinator.h"
    #include "generic_devices.h"
    #include "json.h"
}

#include "drivers/drivers.h"




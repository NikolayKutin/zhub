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
#include <cmath>

#include <boost/signals2.hpp>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/bimap.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

using namespace std::chrono;

namespace utils {

    #include "../utils/event_emitter.h"
    #include "../utils/data.h"
    #include "../utils/factory.h"
	#include "../utils/factory_class.h"
}
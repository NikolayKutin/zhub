// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define LOWBYTE(a) (static_cast<uint8_t>(a))
#define HIGHBYTE(a) static_cast<uint8_t>((static_cast<uint16_t>((a)) >> 8))
#define _UINT16(a, b) (static_cast<uint16_t>(a) + (static_cast<uint16_t>(b) << 8))

template <typename T> std::vector<uint8_t> to_uint8_vector(T value) { return std::vector<uint8_t>(reinterpret_cast<uint8_t*>(&value), reinterpret_cast<uint8_t*>(&value) + sizeof(T)); }
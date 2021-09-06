// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string>
#include "factory.h"

template <class Base, typename ... ConstuctorArgs> class FactoryClass {
public:
	static std::shared_ptr<Base> createInstance(std::string type, ConstuctorArgs ... args) { return factory_.get(type)(args ...); }
	template <class T> static void registerClass(std::string type) { factory_.add<T>(type); }

protected:
	static utils::GenericObjectFactory<std::string, Base, ConstuctorArgs ...> factory_;
};
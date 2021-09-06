// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#include "zigbee.h"

std::string zigbee::toJson(const EndDevice& device) {
	std::stringstream sstream;

	zigbee::EndDevice::Descriptor device_descriptor = device.getDescriptor();
	sstream << "{\"nwk_addr\": 0x" << std::hex << device_descriptor.network_address <<
		", \"ieee_addr\": 0x" << device_descriptor.ieee_address << ", \"model\": \"" << device_descriptor.model << "\"}";

	return std::move(sstream.str());
}

std::string zigbee::toJson(const SimpleDescriptor& descriptor) {
	std::stringstream sstream;

	sstream << "{\"num\": " << std::dec << descriptor.endpoint_number << ", \"profile_id\": 0x" << std::hex << descriptor.profile_id
		<< ", \"device_id\": 0x" << descriptor.device_id << ", \"device_ver\": 0x" << descriptor.device_version << ", \"input_clusters\": [";
	for (auto it = descriptor.input_clusters.begin(); it != descriptor.input_clusters.end();) {
		sstream << "0x" << *it;
		if (++it != descriptor.input_clusters.end()) sstream << ", ";
	}

	sstream << "], \"output_clusters\": [";
	for (auto it = descriptor.output_clusters.begin(); it != descriptor.output_clusters.end();) {
		sstream << "0x" << *it;
		if (++it != descriptor.output_clusters.end()) sstream << ", ";
	}
	sstream << "]}";

	return std::move(sstream.str());
}
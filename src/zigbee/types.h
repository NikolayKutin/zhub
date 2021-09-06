// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

typedef uint16_t NetworkAddress;
typedef uint64_t MacAddress;

struct SimpleDescriptor {
	unsigned int endpoint_number = 0;
	unsigned int profile_id = 0;
	unsigned int device_id = 0;
	unsigned int device_version = 0;
	std::vector<unsigned int> input_clusters;
	std::vector<unsigned int> output_clusters;
};

struct Endpoint {
	NetworkAddress address;
	uint8_t number;

	bool operator==(const Endpoint &endpoint) const { return (this->address == endpoint.address && this->number == endpoint.number); }
	bool operator!=(const Endpoint &endpoint) const { return (!(*this == endpoint)); }
};

struct Message {
	Endpoint source;
	Endpoint destination;
	zcl::Cluster cluster;
	zcl::Frame zcl_frame;
};

enum class LogicalType {
	COORDINATOR = 0,
	ROUTER = 1,
	END_DEVICE = 2
}; // TODO: Znp?

struct NetworkConfiguration {
	uint16_t pan_id = 0;
	uint64_t extended_pan_id = 0;
	LogicalType logical_type = LogicalType::COORDINATOR;
	std::vector<unsigned int> channels;
	std::array<uint8_t, 16> precfg_key = {};
	bool precfg_key_enable = false; // value: 0 (FALSE) only coord defualtKey need to be set, and OTA to set other devices in the network.
									// value: 1 (TRUE) Not only coord, but also all devices need to set their defualtKey (the same key). Or they can't not join the network.

	bool operator==(const NetworkConfiguration &configuration) const {
		return (this->pan_id == configuration.pan_id && 
				this->extended_pan_id == configuration.extended_pan_id && 
				this->logical_type == configuration.logical_type && 
				this->channels == configuration.channels && 
				precfg_key_enable ? this->precfg_key == configuration.precfg_key : true);
	}

	bool operator!=(const NetworkConfiguration &configuration) const { return (!(*this == configuration)); }
};

class Device {
public:

	virtual zigbee::NetworkAddress getNetworkAddress() = 0;
	virtual zigbee::MacAddress getMacAddress() = 0;
};
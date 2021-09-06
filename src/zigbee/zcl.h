// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace zcl {
	enum class Cluster : uint16_t {
		BASIC = 0x0000, ON_OFF = 0x0006, LEVEL_CONTROL = 0x0008,
		TEMPERATURE_MEASUREMENT = 0x402, HUMIDITY_MEASUREMENT = 0x405,
		ILLUMINANCE_MEASUREMENT = 0x0400, POWER_CONFIGURATION = 0x0001,
		MULTISTATE_INPUT = 0x0012, ANALOG_INPUT = 0x000c
	};

	enum class FrameType : uint8_t { GLOBAL = 0, SPECIFIC = 1 };
	enum class FrameDirection : uint8_t { FROM_CLIENT_TO_SERVER = 0, FROM_SERVER_TO_CLIENT = 8 };

	enum Status : uint8_t {
		SUCCESS = 0x00,
		FAILURE = 0x01,
		UNSUPPORTED_ATTRIBUTE = 0x86,
		INVALID_DATA_TYPE = 0x8d,
		NOT_AUTHORIZED = 0x7e,
		RESERVED_FIELD_NOT_ZERO = 0x7f,
		MALFORMED_COMMAND = 0x80,
		UNSUP_CLUSTER_COMMAND = 0x81,
		UNSUPPORTED_GENERAL_COMMAND = 0x82,
		UNSUPPORTED_MANUF_CLUSTER_COMMAND = 0x83,
		UNSUPPORTED_MANUF_GENERAL_COMMAND = 0x84,
		INVALID_FIELD = 0x85,
		INVALID_VALUE = 0x87,
		READ_ONLY = 0x88,
		INSUFFICIENT_SPACE = 0x89,
		DUPLICATE_EXISTS = 0x8a,
		NOT_FOUND = 0x8b,
		UNREPORTABLE_ATTRIBUTE = 0x8c,
		INVALID_SELECTOR = 0x8e,
		WRITE_ONLY = 0x8f,
		INCONSISTENT_STARTUP_STATE = 0x90,
		DEFINED_OUT_OF_BAND = 0x91,
		INCONSISTENT = 0x92,
		ACTION_DENIED = 0x93,
		TIMEOUT = 0x94,
		ABORT = 0x95,
		INVALID_IMAGE = 0x96,
		WAIT_FOR_DATA = 0x97,
		NO_IMAGE_AVAILABLE = 0x98,
		REQUIRE_MORE_IMAGE = 0x99,
		NOTIFICATION_PENDING = 0x9a,
		HARDWARE_FAILURE = 0xc0,
		SOFTWARE_FAILURE = 0xc1,
		CALIBRATION_ERROR = 0xc2,
		UNSUPPORTED_CLUSTER = 0xc3
	};

	struct Frame {
		struct {
			FrameType type;
			bool manufacturer_specific;
			FrameDirection direction;
			bool disable_default_response;
		} frame_control;

		uint16_t manufacturer_code;
		uint8_t transaction_sequence_number;
		uint8_t command;

		std::vector<uint8_t> payload;
	};

	enum GlobalCommands : uint8_t { READ_ATTRIBUTES = 0x00, READ_ATTRIBUTES_RESPONSE = 0x01, WRITE_ATTRIBUTES = 0x02, WRITE_ATTRIBUTES_RESPONSE = 0x04, REPORT_ATTRIBUTES = 0x0a, DEFAULT_RESPONSE = 0x0b, CONFIGURE_REPORTING = 0x06, CONFIGURE_REPORTING_RESPONSE = 0x07 };

}
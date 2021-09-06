// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace zcl {

    template <typename T> std::vector<uint8_t> convertAttributeValue(std::any value) { return utils::to_uint8_vector<T>(std::any_cast<T>(value)); }
	template <typename T> T convertAttributeData(uint8_t *data) { return *reinterpret_cast<T *>(data); }
	
	struct Attribute {
		uint16_t id;
		std::any value;

		enum class DataType { 	NODATA = 0x00,
								// DATA8 = 0x08, DATA16 = 0x09, DATA24 = 0x0a, DATA32 = 0x0b, DATA40 = 0x0c, DATA48 = 0x0d, DATA56 = 0x0e, DATA64 = 0x0f,
								BOOLEAN = 0x10,
								// BITMAP8 = 0x18, BITMAP16 = 0x19, BITMAP24 = 0x1a, BITMAP32 = 0x1b, BITMAP40 = 0x1c, BITMAP48 = 0x1d, BITMAP56 = 0x1e, BITMAP64 = 0x1f,
								UINT8 = 0x20, UINT16 = 0x21, UINT24 = 0x22, UINT32 = 0x23, UINT40 = 0x24, UINT48 = 0x25, UINT56 = 0x26, UINT64 = 0x27,
								INT8 = 0x28, INT16 = 0x29, INT24 = 0x2a, INT32 = 0x2b, INT40 = 0x2c, INT48 = 0x2d, INT56 = 0x2e, INT64 = 0x2f,
								// ENUM8 = 0x30, ENUM16 = 0x31,
								SEMI_FLOAT = 0x38, FLOAT = 0x39, DOUBLE = 0x3a,
								CHARACTER_STRING = 0x42,
								STRUCTURE = 0x4c 
		} data_type;

		std::vector<uint8_t> data();

		static std::vector<Attribute> parseAttributesPayload(std::vector<uint8_t> &payload, bool status_field);

	};

	namespace Attributes {
		enum Basic : uint16_t {	ZCL_VERSION				= 0x0000,	// Type: uint8, Range: 0x00 – 0xff, Access : Read Only, Default: 0x02.
								APPLICATION_VERSION		= 0x0001,	// Type: uint8, Range: 0x00 – 0xff, Access : Read Only, Default: 0x00.
								STACK_VERSION			= 0x0002,	// Type: uint8, Range: 0x00 – 0xff, Access : Read Only, Default: 0x00.
								HW_VERSION				= 0x0003,	// Type: uint8, Range: 0x00 – 0xff, Access : Read Only, Default: 0x00.
								MANUFACTURER_NAME		= 0x0004,	// Type: string, Range: 0 – 32 bytes, Access : Read Only, Default: Empty string.
								MODEL_IDENTIFIER		= 0x0005,	// Type: string, Range: 0 – 32 bytes, Access : Read Only, Default: Empty string.
								DATA_CODE				= 0x0006,	// Type: string, Range: 0 – 16 bytes, Access : Read Only, Default: Empty string.
								POWER_SOURCE			= 0x0007,	// Type: enum8, Range: 0x00 – 0xff, Access : Read Only, Default: 0x00.
								LOCATION_DESCRIPTION	= 0x0010,	// Type: string, Range: 0 – 16 bytes, Access : Read Write, Default: Empty string.
								PHYSICAL_ENVIRONMENT	= 0x0011,	// Type: enum8, Range: 0x00 – 0xff, Access : Read Write, Default: 0x00.
								DEVICE_ENABLED			= 0x0012,	// Type: bool, Range: 0x00 – 0x01, Access : Read Write, Default: 0x01.
								ALARM_MASK				= 0x0013,	// Type: map8, Range: 000000xx, Access : Read Write, Default: 0x00.
								DISABLE_LOCAL_CONFIG	= 0x0014,	// Type: map8, Range: 000000xx, Access : Read Write, Default: 0x00.
								SW_BUILD_ID				= 0x4000,	// Type: string, Range: 0 – 16 bytes, Access : Read Only, Default: Empty string.
		};

		enum OnOffSwitch : uint16_t { 
								ON_OFF = 0x0000,				// Type: bool, Range: 0x00 – 0x01, Access : Read Only, Default: 0x00.
								GLOBAL_SCENE_CONTROL = 0x4000,	// Type: bool, Range: 0x00 – 0x01, Access : Read Only, Default: 0x01.
								ON_TIME = 0x4001,				// Type: uint16, Range: 0x0000 – 0xffff, Access : Read Write, Default: 0x0000.
								OFF_WAIT_TIME = 0x4002			// Type: uint16, Range: 0x0000 – 0xffff, Access : Read Write, Default: 0x0000.
		};

		enum IlluminanceMeasurement : uint16_t {
			ILLUM_MEASURED_VALUE		 = 0x0000,	// Type: uint16, Range: 0x0000 – 0xffff, Access : Read Only.
			ILLUM_MIN_MEASURED_VALUE	 = 0x0001,	// Type: uint16, Range: 0x0000 – 0xffff, Access : Read Only.
			ILLUM_MAX_MEASURED_VALUE	 = 0x0002,	// Type: uint16, Range: 0x0000 – 0xffff, Access : Read Only.
			ILLUM_TOLERANCE			 = 0x0003,	// Type: uint16, Range: 0x0000 – 0xffff, Access : Read Only.
			LIGHT_SENSOR_TYPE	 = 0x0004	// Type: enum8, Range: 0x00 – 0xff, Access : Read Only.
		};

		enum MultistateInput : uint16_t {
			PRESENT_VALUE = 0x0055,			// Type: uint16, Range: 0x0000 – 0xffff, Access : Read Write.
		};

		enum TemperatureMeasurement : uint16_t {
			TEMP_MEASURED_VALUE = 0x0000,		// Type: int16, Range: 0x0000 – 0xffff, Access : Read Only.
			TEMP_MIN_MEASURED_VALUE = 0x0001,	// Type: int16, Range: 0x954d – 0x7ffe, Access : Read Only.
			TEMP_MAX_MEASURED_VALUE = 0x0002,	// Type: int16, Range: 0x954e – 0x7fff, Access : Read Only.
			TEMP_TOLERANCE = 0x0003,				// Type: uint16, Range: 0x0000 – 0x0800, Access : Read Only.
		};

		enum RelativeHumidityMeasurement : uint16_t {
			HUM_MEASURED_VALUE = 0x0000,		// Type: uint16, Range: 0x0000 – 0xffff, Access : Read Only.
			HUM_MIN_MEASURED_VALUE = 0x0001,	// Type: uint16, Range: 0x954d – 0x7ffe, Access : Read Only.
			HUM_MAX_MEASURED_VALUE = 0x0002,	// Type: uint16, Range: 0x954e – 0x7fff, Access : Read Only.
			HUM_TOLERANCE = 0x0003,				// Type: uint16, Range: 0x0000 – 0x0800, Access : Read Only.
		};

		enum AnalogInput : uint16_t {
			ANALOG_PRESENT_VALUE = 0x0055,				// Type: float, Range: -, Access : Read Only.
		};
	}

	enum class PowerSource : uint8_t { UNKNOWN = 0x0000, SINGLE_PHASE = 0x01, THREE_PHASE = 0x02, BATTARY = 0x03, DC = 0x04, EMERGENCY_CONSTANTLY = 0x05, EMERGENCY_TRANSFER_SWITCH = 0x06 };

	
}
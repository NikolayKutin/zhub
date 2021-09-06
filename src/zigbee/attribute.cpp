// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#include "zigbee.h"

using namespace zigbee;

std::vector<zcl::Attribute> zcl::Attribute::parseAttributesPayload(std::vector<uint8_t> &payload, bool status_field) { // TODO: Add attributes types.
    std::vector<Attribute> attributes;

    try {
        size_t i = 0;
        
        while (i < payload.size()) {
            zcl::Attribute attribute;

            attribute.id = _UINT16(payload.at(i++), payload.at(i++));

            if (status_field) {
                Status attribute_status = static_cast<Status>(payload.at(i++));

                if (attribute_status != Status::SUCCESS)
                    if (attributes.empty())
                        throw(attribute_status);
                    else
                        break;
		    }

            attribute.data_type = static_cast<DataType>(payload.at(i++));

            size_t size = 0 ;

            switch (attribute.data_type) {

            case DataType::STRUCTURE: {
                size = _UINT16(payload.at(i++), payload.at(i++));
                attribute.value = std::vector<uint8_t>( reinterpret_cast<uint8_t *>(payload.data() + i),
                                                        reinterpret_cast<uint8_t *>(payload.data() + i + size));
                break;
            }

            case DataType::CHARACTER_STRING: {
                size = payload.at(i++);
                attribute.value = std::string(reinterpret_cast<char *>(payload.data() + i), size);
                break;
            }

            case DataType::BOOLEAN: { attribute.value = convertAttributeData<bool>              (payload.data() + i); size = sizeof(bool);              break; }
            case DataType::UINT8:   { attribute.value = convertAttributeData<uint8_t>           (payload.data() + i); size = sizeof(uint8_t);           break; }
            case DataType::UINT16:  { attribute.value = convertAttributeData<uint16_t>          (payload.data() + i); size = sizeof(uint16_t);          break; }
            case DataType::INT16:   { attribute.value = convertAttributeData<int16_t>           (payload.data() + i); size = sizeof(int16_t);           break; }
            case DataType::UINT32:  { attribute.value = convertAttributeData<uint32_t>          (payload.data() + i); size = sizeof(uint32_t);          break; }
            case DataType::FLOAT:   { attribute.value = convertAttributeData<boost::float32_t>  (payload.data() + i); size = sizeof(boost::float32_t);  break; }

            default: throw std::invalid_argument(std::string("Unknown attribute data type: ") + std::to_string((unsigned)attribute.data_type));
            }

            attributes.push_back(attribute);
            i += size;     
        }
    }
    catch (std::out_of_range) {
        if(attributes.empty())
            throw(std::runtime_error("Invalid attribute pyload"));
    }

    return attributes;
}

std::vector<uint8_t> zcl::Attribute::data() {
	std::vector<uint8_t> attribute_data;

    try {
        
        switch(data_type) {

        case DataType::CHARACTER_STRING: { // TODO: types
            // TODO: assert
            std::string character_string = std::any_cast<std::string>(value);

            attribute_data.push_back(static_cast<uint8_t>(character_string.size()));

            std::copy_n(reinterpret_cast<uint8_t *>(character_string.data()), character_string.size(), std::back_inserter(attribute_data));
            break;
        }

        case DataType::STRUCTURE: {
            // TODO: assert
            std::vector<uint8_t> structure_data = std::any_cast<std::vector<uint8_t>>(value);

            attribute_data.push_back(LOWBYTE(structure_data.size()));
            attribute_data.push_back(HIGHBYTE(structure_data.size()));

            std::copy(structure_data.begin(), structure_data.end(), std::back_inserter(attribute_data));
            break;
        }

        case DataType::BOOLEAN: { attribute_data = convertAttributeValue<bool>(value); break; }
        case DataType::UINT8: { attribute_data = convertAttributeValue<uint8_t>(value); break; }
        case DataType::UINT16: { attribute_data = convertAttributeValue<uint16_t>(value); break; }
        case DataType::INT16: { attribute_data = convertAttributeValue<int16_t>(value); break; }

        default: throw std::invalid_argument("Unknown attribute data type");
        }
   	}
    catch (std::bad_any_cast) { throw std::invalid_argument("Invalid attribute data type"); }

	return attribute_data;
}
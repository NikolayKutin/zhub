// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#include <fstream>
#include <yaml-cpp/yaml.h>
#include "zhub.h"

void Zhub::loadConfigurationFromYaml(const std::string& config_file_name) {
    try {
        YAML::Node yaml_root_node = YAML::LoadFile(config_file_name);

        configuration_.zigbee.serial = yaml_root_node["zigbee"]["serial"].as<std::string>();
        configuration_.zigbee.baudrate = yaml_root_node["zigbee"]["baudrate"].as<unsigned int>();

        configuration_.mqtt.host = yaml_root_node["mqtt"]["host"].as<std::string>();
        configuration_.mqtt.port = yaml_root_node["mqtt"]["port"].as<unsigned>();
        configuration_.mqtt.user_name = yaml_root_node["mqtt"]["user_name"].as<std::string>();
        configuration_.mqtt.password = yaml_root_node["mqtt"]["password"].as<std::string>();
        configuration_.mqtt.root_topic = yaml_root_node["mqtt"]["root_topic"].as<std::string>();

        for (auto unit_subnode : yaml_root_node["units"]) {
            FunctionalUnit::Descriptor descriptor;

            descriptor.type = unit_subnode["type"].as<std::string>();
            descriptor.id = unit_subnode["id"].as<std::string>();

            for (auto device_subnode : unit_subnode["devices"]) {
                zigbee::EndDevice::Descriptor device_descriptor;

                device_descriptor.model = device_subnode["model"].as<std::string>();
                device_descriptor.network_address = device_subnode["network_address"].as<uint16_t>();
                device_descriptor.ieee_address = device_subnode["ieee_address"].as<uint64_t>();

                descriptor.devices.push_back(std::move(device_descriptor));
            }

            for (auto option_subnode : unit_subnode["options"])
                descriptor.options.push_back(std::make_pair(option_subnode.first.as<std::string>(), option_subnode.second.as<std::string>()));

            configuration_.unit_descriptors.push_back(std::move(descriptor));
        }
    }
    catch (YAML::Exception error) {
        throw std::runtime_error(std::string("Configuration loading error (") + error.what() + ")");
    }
}

void Zhub::saveConfigurationToYaml(const std::string& config_file_name) {
    YAML::Node yaml_root_node;

    yaml_root_node["zigbee"]["serial"] = configuration_.zigbee.serial;
    yaml_root_node["zigbee"]["baudrate"] = configuration_.zigbee.baudrate;

    yaml_root_node["mqtt"]["host"] = configuration_.mqtt.host;
    yaml_root_node["mqtt"]["port"] = configuration_.mqtt.port;
    yaml_root_node["mqtt"]["user_name"] = configuration_.mqtt.user_name;
    yaml_root_node["mqtt"]["password"] = configuration_.mqtt.password;
    yaml_root_node["mqtt"]["root_topic"] = configuration_.mqtt.root_topic;

    size_t i = 0;
    for (auto unit_descriptor : configuration_.unit_descriptors) {
        yaml_root_node["units"][i]["type"] = unit_descriptor.type;
        yaml_root_node["units"][i]["id"] = unit_descriptor.id;

        for (auto option : unit_descriptor.options)
            yaml_root_node["units"][i]["options"][option.first] = option.second;

        size_t j = 0;
        for (auto device_descriptor : unit_descriptor.devices) {
            yaml_root_node["units"][i]["devices"][j].SetStyle(YAML::EmitterStyle::Flow);

            yaml_root_node["units"][i]["devices"][j]["network_address"] = device_descriptor.network_address;
            yaml_root_node["units"][i]["devices"][j]["ieee_address"] = device_descriptor.ieee_address;
            yaml_root_node["units"][i]["devices"][j]["model"] = device_descriptor.model;

            j++;
        }

        i++;
    }

    std::ofstream config_file(config_file_name); // TODO error handling
    config_file << yaml_root_node;
    config_file.close();
}
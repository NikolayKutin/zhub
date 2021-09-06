// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

class WSDCGQ01LM : public GenericDevices::TemperatureSensor, public GenericDevices::HumiditySensor {
public:

    WSDCGQ01LM() = delete;
    WSDCGQ01LM(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_adress, zigbee::Coordinator* coordinator) :
        TemperatureSensor(network_address, mac_adress, coordinator), HumiditySensor(network_address, mac_adress, coordinator),
        zigbee::EndDevice(network_address, mac_adress, coordinator) {}
    ~WSDCGQ01LM() {}

    std::string model() const { return "WSDCGQ01LM"; }
    std::vector<zigbee::EndDevice::Type> types() const { return { zigbee::EndDevice::Type::TEMPERATURE_SENSOR, zigbee::EndDevice::Type::HUMIDITY_SENSOR }; }

private:

    void onAttributeReport(uint8_t endpiont_number, zcl::Cluster cluster, zcl::Attribute attribute) {
        TemperatureSensor::onAttributeReport(endpiont_number, cluster, attribute);
        HumiditySensor::onAttributeReport(endpiont_number, cluster, attribute);
    }
};
// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

class LED1623G12 : public GenericDevices::OnOffSwitch, public GenericDevices::LevelControledDevice {
public:

    LED1623G12() = delete;
    LED1623G12(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_adress, zigbee::Coordinator* coordinator) : OnOffSwitch(network_address, mac_adress, coordinator), LevelControledDevice(network_address, mac_adress, coordinator), zigbee::EndDevice(network_address, mac_adress, coordinator) {}
    ~LED1623G12() {}

    std::string model() const { return "LED1623G12"; }
    std::vector<zigbee::EndDevice::Type> types() const { return { zigbee::EndDevice::Type::ON_OFF, zigbee::EndDevice::Type::LEVEL_CONTROL }; }

    void on() { OnOffSwitch::on(); OnStateChange(GenericDevices::OnOffSwitch::ON); }
    void off() { OnOffSwitch::off(); OnStateChange(GenericDevices::OnOffSwitch::OFF); }

private:

    void onAttributeReport(uint8_t endpiont_number, zcl::Cluster cluster, zcl::Attribute attribute) {}
};
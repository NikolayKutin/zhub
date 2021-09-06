// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

class BASICZBR3 : public GenericDevices::OnOffSwitch {
public:

    BASICZBR3() = delete;
    BASICZBR3(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_adress, zigbee::Coordinator* coordinator) : OnOffSwitch(network_address, mac_adress, coordinator), zigbee::EndDevice(network_address, mac_adress, coordinator) {}
    ~BASICZBR3() {}

    std::string model() const { return "BASICZBR3"; }
    std::vector<zigbee::EndDevice::Type> types() const { return { zigbee::EndDevice::Type::ON_OFF }; }

private:

    void onAttributeReport(uint8_t endpiont_number, zcl::Cluster cluster, zcl::Attribute attribute) {
        OnOffSwitch::onAttributeReport(endpiont_number, cluster, attribute);
    }
};
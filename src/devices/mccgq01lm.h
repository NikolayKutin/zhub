// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

class MCCGQ01LM : public GenericDevices::Trigger {
public:

    MCCGQ01LM() = delete;
    MCCGQ01LM(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_adress, zigbee::Coordinator* coordinator) :
        Trigger(network_address, mac_adress, coordinator), zigbee::EndDevice(network_address, mac_adress, coordinator) {}
    ~MCCGQ01LM() {}

    std::string model() const { return "MCCGQ01LM"; }
    std::vector<zigbee::EndDevice::Type> types() const { return { zigbee::EndDevice::Type::TRIGGER }; }

private:

    void onAttributeReport(uint8_t endpiont_number, zcl::Cluster cluster, zcl::Attribute attribute) {
        Trigger::onAttributeReport(endpiont_number, cluster, attribute);
    }
};
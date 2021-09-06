// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

class QBKG11LM : public GenericDevices::OnOffSwitch {
public:

    QBKG11LM() = delete;
    QBKG11LM(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_adress, zigbee::Coordinator* coordinator) : OnOffSwitch(network_address, mac_adress, coordinator), zigbee::EndDevice(network_address, mac_adress, coordinator) {}
    ~QBKG11LM() {}

    std::string model() const { return "QBKG11LM"; }
    std::vector<zigbee::EndDevice::Type> types() const { return { zigbee::EndDevice::Type::ON_OFF }; }

private:

    void onAttributeReport(uint8_t endpiont_number, zcl::Cluster cluster, zcl::Attribute attribute) {
        if ((cluster == zcl::Cluster::ANALOG_INPUT) && (attribute.id = zcl::Attributes::AnalogInput::ANALOG_PRESENT_VALUE)) {
            try {
                OnPowerMonitor(std::any_cast<boost::float32_t>(attribute.value)); // post ?
            }
            catch (std::bad_any_cast) {} // TODO: error handling
        }
        else
            OnOffSwitch::onAttributeReport(endpiont_number, cluster, attribute);
    }
};
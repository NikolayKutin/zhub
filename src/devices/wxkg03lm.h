// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

class WXKG03LM : public GenericDevices::Button {
public:

    WXKG03LM() = delete;
    WXKG03LM(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_adress, zigbee::Coordinator* coordinator) : Button(network_address, mac_adress, coordinator), zigbee::EndDevice(network_address, mac_adress, coordinator) {}
    ~WXKG03LM() {}

    std::string model() const { return "WXKG03LM"; }
    std::vector<zigbee::EndDevice::Type> types() const { return { zigbee::EndDevice::Type::BUTTON }; }

private:

    void onAttributeReport(uint8_t endpiont_number, zcl::Cluster cluster, zcl::Attribute attribute) {
        if ((cluster == zcl::Cluster::MULTISTATE_INPUT) && (attribute.id == zcl::Attributes::MultistateInput::PRESENT_VALUE)) {
            try {
                (std::any_cast<uint16_t>(attribute.value) == 1) ? OnClick() : OnMultiClick();
            }
            catch (std::bad_any_cast) { return; } // TODO: error handling
        }
    }
};

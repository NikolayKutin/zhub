// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

class WXKG01LM : public GenericDevices::Button {
public:

    WXKG01LM() = delete;
    WXKG01LM(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_adress, zigbee::Coordinator* coordinator) : Button(network_address, mac_adress, coordinator), zigbee::EndDevice(network_address, mac_adress, coordinator) {}
    ~WXKG01LM() {}

    std::string model() const { return "WXKG01LM"; }
    std::vector<zigbee::EndDevice::Type> types() const { return { zigbee::EndDevice::Type::BUTTON }; }

private:

    void onAttributeReport(uint8_t endpiont_number, zcl::Cluster cluster, zcl::Attribute attribute) {
        if ((cluster == zcl::Cluster::ON_OFF) && (attribute.id == 0x8000))
            OnMultiClick(); // coordinator_->getIoService()->post(boost::ref(OnMultiClick)); break;
        else
            Button::onAttributeReport(endpiont_number, cluster, attribute);
    }
};
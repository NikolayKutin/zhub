// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

class GZCGQ01LM : public GenericDevices::IlluminanceLevelSensor {
public:

    GZCGQ01LM() = delete;
    GZCGQ01LM(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_adress, zigbee::Coordinator* coordinator) : IlluminanceLevelSensor(network_address, mac_adress, coordinator), zigbee::EndDevice(network_address, mac_adress, coordinator) {}
    ~GZCGQ01LM() {}

    std::string model() const { return "GZCGQ01LM"; }
    std::vector<zigbee::EndDevice::Type> types() const { return { zigbee::EndDevice::Type::ILLUMINANCE_LEVEL_SENSOR }; }

    void configure() {
        bind(1, 1, zcl::Cluster::POWER_CONFIGURATION);
        bind(1, 1, zcl::Cluster::ILLUMINANCE_MEASUREMENT);

        configureReporting(1, zcl::Cluster::ILLUMINANCE_MEASUREMENT, zcl::Attributes::IlluminanceMeasurement::ILLUM_MEASURED_VALUE,
            zcl::Attribute::DataType::UINT16, 15s, 1h, GR_REP_CHANGE(uint16_t, 500));
    }

private:

    void onAttributeReport(uint8_t endpiont_number, zcl::Cluster cluster, zcl::Attribute attribute) {
        IlluminanceLevelSensor::onAttributeReport(endpiont_number, cluster, attribute);
    }
};
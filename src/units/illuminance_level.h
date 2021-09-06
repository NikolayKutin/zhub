// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

class IlluminanceLevel : virtual public FunctionalUnit {
public:
    IlluminanceLevel() = delete;
    IlluminanceLevel(std::string id, zigbee::Coordinator* coordinator) : FunctionalUnit(id, coordinator) {
        BIND_ATTACH_HANDLER(zigbee::EndDevice::Type::ILLUMINANCE_LEVEL_SENSOR, IlluminanceLevel::attachIlluminanceSensor);

        BIND_CHANNEL_HANDLER("illuminance", IlluminanceLevel::illuminanceChannelCommandHandler);
    }

    virtual ~IlluminanceLevel() {}

    std::string type() { return std::string("illuminance"); }

    std::vector<std::pair<std::string, std::string>> options() { return std::vector<std::pair<std::string, std::string>>(); }

    virtual void publicAll() {
        if(illuminance_sensor_) publicIlluminance(illuminance_sensor_->getIlluminanceLevel());
    }

protected:

    void attachIlluminanceSensor(std::shared_ptr<zigbee::EndDevice> device) {
        illuminance_sensor_ = std::dynamic_pointer_cast<zigbee::GenericDevices::IlluminanceLevelSensor>(device);

        illuminance_sensor_->OnIlluminanceChange.connect(boost::bind(&IlluminanceLevel::publicIlluminance, this, boost::placeholders::_1));
    }

    void publicIlluminance(unsigned illuminance) { OnMessage(this, "illuminance", std::to_string(illuminance)); }

    void illuminanceChannelCommandHandler(std::string command) {
        if (command == "?" && illuminance_sensor_) publicIlluminance(illuminance_sensor_->getIlluminanceLevel());
    }

    virtual void removeDevices() {
        illuminance_sensor_->OnIlluminanceChange.disconnect_all_slots();
        illuminance_sensor_.reset();

        FunctionalUnit::removeDevices();
    }

    std::shared_ptr<zigbee::GenericDevices::IlluminanceLevelSensor> illuminance_sensor_;
};
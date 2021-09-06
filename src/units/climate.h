// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

class Climate : virtual public FunctionalUnit {
public:
    Climate() = delete;
    Climate(std::string id, zigbee::Coordinator* coordinator) : FunctionalUnit(id, coordinator) {
        BIND_ATTACH_HANDLER(zigbee::EndDevice::Type::TEMPERATURE_SENSOR, Climate::attachTemperatureSensor);
        BIND_ATTACH_HANDLER(zigbee::EndDevice::Type::HUMIDITY_SENSOR, Climate::attachHumiditySensor);

        BIND_CHANNEL_HANDLER("temperature", Climate::temperatureChannelCommandHandler);
        BIND_CHANNEL_HANDLER("humidity", Climate::humidityChannelCommandHandler);

        OnTemperatureReport.connect(boost::bind(&Climate::publicTemperature, this, boost::placeholders::_1));
        OnHumidityReport.connect(boost::bind(&Climate::publicHumidity, this, boost::placeholders::_1));
    }

    virtual ~Climate() {}

    std::string type() { return std::string("climate"); }

    std::vector<std::pair<std::string, std::string>> options() { return std::vector<std::pair<std::string, std::string>>(); }

    virtual void publicAll() {
        if(temperature_sensor_) publicTemperature(temperature_sensor_->getTemperature());
        if (humidity_sensor_) publicHumidity(humidity_sensor_->getHumidity());
    }

protected:

    boost::signals2::signal<void(float)> OnTemperatureReport;
    boost::signals2::signal<void(float)> OnHumidityReport;

    void publicTemperature(float temperature) { OnMessage(this, "temperature", std::to_string(temperature)); };
    void publicHumidity(float humidity) { OnMessage(this, "humidity", std::to_string(humidity)); };

    void temperatureChannelCommandHandler(std::string command) {
        if (command == "?" && temperature_sensor_) publicTemperature(temperature_sensor_->getTemperature());
    }

    void humidityChannelCommandHandler(std::string command) {
        if (command == "?" && humidity_sensor_) publicHumidity(humidity_sensor_->getHumidity());
    }

    void attachTemperatureSensor(std::shared_ptr<zigbee::EndDevice> device) {
        temperature_sensor_ = std::dynamic_pointer_cast<zigbee::GenericDevices::TemperatureSensor>(device);

        temperature_sensor_->OnTemperatureChange.connect(OnTemperatureReport);
    }

    void attachHumiditySensor(std::shared_ptr<zigbee::EndDevice> device) {
        humidity_sensor_ = std::dynamic_pointer_cast<zigbee::GenericDevices::HumiditySensor>(device);

        humidity_sensor_->OnHumidityChange.connect(OnHumidityReport);
    }

    virtual void removeDevices() {
        temperature_sensor_->OnTemperatureChange.disconnect_all_slots();
        temperature_sensor_.reset();

        humidity_sensor_->OnHumidityChange.disconnect_all_slots();
        humidity_sensor_.reset();

        FunctionalUnit::removeDevices();
    }

    std::shared_ptr<zigbee::GenericDevices::TemperatureSensor> temperature_sensor_;
    std::shared_ptr<zigbee::GenericDevices::HumiditySensor> humidity_sensor_;
};
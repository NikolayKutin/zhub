// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define ADD_ZIGBEE_DEVICE_CLASS(class_, identifier, model) zigbee::EndDevice::registerDeviceClass<class_>(identifier, model)

namespace GenericDevices {

    class Trigger : virtual public zigbee::EndDevice {
    public:
        enum State : bool { RELEASED = false, TRIGGERED = true } ;

        boost::signals2::signal<void(State)> OnTriggered;

    protected:

        Trigger() = delete;
        Trigger(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_adress, zigbee::Coordinator* coordinator) : zigbee::EndDevice(network_address, mac_adress, coordinator) {}
        ~Trigger() {}

        virtual void onAttributeReport(uint8_t endpiont_number, zcl::Cluster cluster, zcl::Attribute attribute) {
            if ((cluster == zcl::Cluster::ON_OFF) && (attribute.id == zcl::Attributes::OnOffSwitch::ON_OFF)) {
                try {
                    OnTriggered((State)std::any_cast<bool>(attribute.value));
                }
                catch (std::bad_any_cast) { return; } // TODO: error handling
            }
        }
    };

    class Button : virtual public zigbee::EndDevice {
    public:
        boost::signals2::signal<void(void)> OnClick;
        boost::signals2::signal<void(void)> OnMultiClick;

    protected:

        Button() = delete;
        Button(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_adress, zigbee::Coordinator* coordinator) : zigbee::EndDevice(network_address, mac_adress, coordinator) {}
        ~Button() {}

        virtual void onAttributeReport(uint8_t endpiont_number, zcl::Cluster cluster, zcl::Attribute attribute) {
            if ((cluster == zcl::Cluster::ON_OFF) && (attribute.id == zcl::Attributes::OnOffSwitch::ON_OFF)) {
                try {
                    if (std::any_cast<bool>(attribute.value))
                        OnClick(); // coordinator_->getIoService()->post(boost::ref(OnClick));
                }
                catch (std::bad_any_cast) { return; } // TODO: error handling
            }
        }
    };

    class OnOffSwitch : virtual public zigbee::EndDevice {
    public:
        enum State { OFF, ON };

        boost::signals2::signal<void(State)> OnStateChange;
        boost::signals2::signal<void(float)> OnPowerMonitor;

        virtual void on() { sendCommand(1, zigbee::zcl::Cluster::ON_OFF, 0x01); }
        virtual void off() { sendCommand(1, zigbee::zcl::Cluster::ON_OFF, 0x00); }
        void toggle() { sendCommand(1, zigbee::zcl::Cluster::ON_OFF, 0x02); }

        State getState() { return static_cast<State>(std::any_cast<bool>(readAttribute(1, zcl::Cluster::ON_OFF, zcl::Attributes::OnOffSwitch::ON_OFF).value)); }
    protected:

        OnOffSwitch() = delete;
        OnOffSwitch(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_adress, zigbee::Coordinator* coordinator) : zigbee::EndDevice(network_address, mac_adress, coordinator) {}
        ~OnOffSwitch() {}

        virtual void onAttributeReport(uint8_t endpiont_number, zcl::Cluster cluster, zcl::Attribute attribute) {
            if (cluster == zcl::Cluster::ON_OFF && attribute.id == zcl::Attributes::OnOffSwitch::ON_OFF)
                try {
                OnStateChange(static_cast<State>(std::any_cast<bool>(attribute.value)));
                // coordinator_->getIoService()->post(boost::bind(boost::ref(OnStateChange), std::any_cast<bool>(attribute.value)));
            }
            catch (std::bad_any_cast error) { return; }
        }
    };

    class LevelControledDevice : virtual public zigbee::EndDevice {
    public:

        void setLevel(uint8_t lvl) { sendCommand(1, zigbee::zcl::Cluster::LEVEL_CONTROL, 0x00, std::vector<uint8_t> { lvl, 0, 0 }); } // TODO: getLevel()

    protected:
        LevelControledDevice() = delete;
        LevelControledDevice(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_adress, zigbee::Coordinator* coordinator) : zigbee::EndDevice(network_address, mac_adress, coordinator) {}
        ~LevelControledDevice() {}
    };

    class IlluminanceLevelSensor : virtual public zigbee::EndDevice {
    public:
        IlluminanceLevelSensor() = delete;
        IlluminanceLevelSensor(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_adress, zigbee::Coordinator* coordinator)
            : zigbee::EndDevice(network_address, mac_adress, coordinator) {}
        ~IlluminanceLevelSensor() {}

        boost::signals2::signal<void(unsigned)> OnIlluminanceChange;
        unsigned getIlluminanceLevel() { return temperature_; }

    protected:

        virtual void onAttributeReport(uint8_t endpiont_number, zcl::Cluster cluster, zcl::Attribute attribute) {
            if ((cluster == zcl::Cluster::ILLUMINANCE_MEASUREMENT) && (attribute.id == zcl::Attributes::IlluminanceMeasurement::ILLUM_MEASURED_VALUE)) {
                try {
                    uint16_t raw_value = std::any_cast<uint16_t>(attribute.value);

                    temperature_ = (unsigned)std::pow(10.0f, ((float)(raw_value) / 10000.0f)) - 1;

                    OnIlluminanceChange(temperature_);
                    // coordinator_->getIoService()->post(boost::bind(boost::ref(OnIlluminanceChange), illuminance_level_));
                }
                catch (std::bad_any_cast) { return; } // TODO: error handling
            }
        }

    private:

        unsigned temperature_ = 0;
    };

    class TemperatureSensor : virtual public zigbee::EndDevice {
    public:
        TemperatureSensor() = delete;
        TemperatureSensor(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_adress, zigbee::Coordinator* coordinator)
            : zigbee::EndDevice(network_address, mac_adress, coordinator) {}
        ~TemperatureSensor() {}

        boost::signals2::signal<void(float)> OnTemperatureChange;
        float getTemperature() { return temperature_; }

    protected:

        virtual void onAttributeReport(uint8_t endpiont_number, zcl::Cluster cluster, zcl::Attribute attribute) {
            if ((cluster == zcl::Cluster::TEMPERATURE_MEASUREMENT) && (attribute.id == zcl::Attributes::TemperatureMeasurement::TEMP_MEASURED_VALUE)) {
                try {
                    int16_t raw_value = std::any_cast<int16_t>(attribute.value);
                    temperature_ = (float)raw_value / 100;

                    OnTemperatureChange(temperature_);
                    // coordinator_->getIoService()->post(boost::bind(boost::ref(OnIlluminanceChange), illuminance_level_));
                }
                catch (std::bad_any_cast) { return; } // TODO: error handling
            }
        }

    private:

        float temperature_ = 0.0f;
    };

    class HumiditySensor : virtual public zigbee::EndDevice {
    public:
        HumiditySensor() = delete;
        HumiditySensor(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_adress, zigbee::Coordinator* coordinator)
            : zigbee::EndDevice(network_address, mac_adress, coordinator) {}
        ~HumiditySensor() {}

        boost::signals2::signal<void(float)> OnHumidityChange;
        float getHumidity() { return humidity_; }

    protected:

        virtual void onAttributeReport(uint8_t endpiont_number, zcl::Cluster cluster, zcl::Attribute attribute) {
            if ((cluster == zcl::Cluster::HUMIDITY_MEASUREMENT) && (attribute.id == zcl::Attributes::RelativeHumidityMeasurement::HUM_MEASURED_VALUE)) {
                try {
                    uint16_t raw_value = std::any_cast<uint16_t>(attribute.value);
                    humidity_ = (float)raw_value / 100;

                    OnHumidityChange(humidity_);
                    // coordinator_->getIoService()->post(boost::bind(boost::ref(OnIlluminanceChange), illuminance_level_));
                }
                catch (std::bad_any_cast) { return; } // TODO: error handling
            }
        }

    private:

        float humidity_ = 0.0f;
    };
}
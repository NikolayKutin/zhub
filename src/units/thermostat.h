// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

class Thermostat : virtual public OnOff, virtual public Climate {
public:
    Thermostat() = delete;
    Thermostat(std::string id, zigbee::Coordinator* coordinator) : FunctionalUnit(id, coordinator), OnOff(id, coordinator),
        Climate(id, coordinator) {
        BIND_CHANNEL_HANDLER("target", Thermostat::targetCommandHandler);
        BIND_CHANNEL_HANDLER("hysteresis", Thermostat::hysteresisCommandHandler);
        BIND_CHANNEL_HANDLER("type", Thermostat::typeCommandHandler);

        OnTemperatureReport.connect(boost::bind(&Thermostat::onTemperatureReport, this, boost::placeholders::_1));
    }

    virtual ~Thermostat() {}

    enum class Type { HEATER, COOLER };

    std::string type() { return std::string("thermostat"); }

    std::vector<std::pair<std::string, std::string>> options() {
        return {    {"type", (type_ == Type::HEATER) ? "heater" : "cooler" },
                    {"hysteresis", std::to_string(hysteresis_)},
                    {"target", std::to_string(target_temperature_)} };
    }

    virtual void publicAll() {
        OnOff::publicAll();
        Climate::publicAll();
    }

protected:

    virtual void removeDevices() {
        OnOff::removeDevices();
        Climate::removeDevices();
    }
 
    void onTemperatureReport(float temperature) {
        current_temperature_ = temperature;
        relayAlgorithm();
    }

    void publicTargetTemperature() { OnMessage(this, "target", std::to_string(target_temperature_)); }
    void publicHysteresis() { OnMessage(this, "hysteresis", std::to_string(hysteresis_)); }
    void publicType() { OnMessage(this, "type", (type_ == Type::HEATER) ? "heater" : "cooler"); }

    void targetCommandHandler(std::string command) {
        if (command == "?") publicTargetTemperature();
        else {
            try {
                target_temperature_ = std::clamp(std::stof(command), 5.0f, 100.0f); // MIN_TEMP = 5 C, MAX_TEMP = 100 C (for sauna)
                relayAlgorithm();
            }
            catch (std::exception error) {
                OnError(this, error.what());  // post?
            }
        }
    }

    void hysteresisCommandHandler(std::string command) {
        if (command == "?") publicHysteresis();
        else {
            try {
                hysteresis_ = std::clamp(std::stof(command), 1.0f, 10.0f); // 1-10 C
                relayAlgorithm();
            }
            catch (std::exception error) {
                OnError(this, error.what());  // post?
            }
        }
    }

    void typeCommandHandler(std::string command) {
        if (command == "?") publicType();
        else if (command == "heater") type_ = Type::HEATER;
        else if (command == "cooler") type_ = Type::COOLER;
    }

    void relayAlgorithm() {
        if (!state_is_actual_) updateState();

        if (switch_)
            try {
                if ((current_temperature_ < (target_temperature_ - hysteresis_)) && state_ == OnOffSwitch::OFF)
                    (type_ == Type::HEATER) ? switch_->on() : switch_->off();
                else if ((current_temperature_ > (target_temperature_ + hysteresis_)) && state_ == OnOffSwitch::ON)
                    (type_ == Type::HEATER) ? switch_->off() : switch_->on();
            }
        catch (std::runtime_error error) {
            OnError(this, error.what());  // post?
        }
    }

    Type type_ = Type::HEATER;

    float current_temperature_ = 0.0f;
    float target_temperature_ = 24.0f;
    float hysteresis_ = 2.0f;
};
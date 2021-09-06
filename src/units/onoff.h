// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

class OnOff : virtual public FunctionalUnit {
public:
    OnOff() = delete;
    OnOff(std::string id, zigbee::Coordinator* coordinator) : FunctionalUnit(id, coordinator) {
        BIND_ATTACH_HANDLER(zigbee::EndDevice::Type::ON_OFF, OnOff::attachSwitch);
        BIND_CHANNEL_HANDLER("power", OnOff::powerChannelCommandHandler);
    }

    virtual ~OnOff() {}

    std::string type() { return std::string("onoff"); }

    std::vector<std::pair<std::string, std::string>> options() { return std::vector<std::pair<std::string, std::string>>(); }

    virtual void publicAll() {
        publicState();
        publicPower();
    }

protected:

    void powerChannelCommandHandler(std::string command) {
        if (switch_) {
            if (!state_is_actual_) updateState();

            if (command == "?") publicState();
            else
                try {
                    if (command == "on" && (state_ == OnOffSwitch::OFF))
                        switch_->on();

                    else if (command == "off" && (state_ == OnOffSwitch::ON))
                        switch_->off();
                }
                catch (std::runtime_error error) {
                    OnError(this, error.what());  // post?
                }
        }
    }

    void onStateChange(OnOffSwitch::State new_state) {
        state_ = new_state;
        state_is_actual_ = true;

        publicState();
    }

    void publicState() { OnMessage(this, "power", state_ ? "on" : "off"); }// post?
    void publicPower() { OnMessage(this, "power/monitor", std::to_string(power_)); }// post?

    void attachSwitch(std::shared_ptr<zigbee::EndDevice> device) {
        switch_ = std::dynamic_pointer_cast<zigbee::GenericDevices::OnOffSwitch>(device);

        switch_->OnStateChange.connect(boost::bind(&OnOff::onStateChange, this, boost::placeholders::_1));

        switch_->OnPowerMonitor.connect([this](float power) { power_ = power;  publicPower(); }); // post?

        updateState();
    }

    void updateState() {
        if (switch_)
            try {
                state_ = switch_->getState();
                state_is_actual_ = true;
            }
        catch (std::runtime_error error) {
            OnError(this, error.what());  // post?
        }
    }

    virtual void removeDevices() {
        switch_->OnStateChange.disconnect_all_slots();
        switch_.reset();

        FunctionalUnit::removeDevices();
    }

    std::shared_ptr<zigbee::GenericDevices::OnOffSwitch> switch_;

    OnOffSwitch::State state_ = OnOffSwitch::OFF;
    bool state_is_actual_ = false;

    float power_ = 0.0f;
};
// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

class Alarm : virtual public FunctionalUnit {
public:

    Alarm() = delete;
    Alarm(std::string id, zigbee::Coordinator* coordinator) : FunctionalUnit(id, coordinator) {
        OnTriggered.connect([this]() {
            state_ = zigbee::GenericDevices::Trigger::TRIGGERED;
            publicState();
            });
        OnReleased.connect([this]() {
            state_ = zigbee::GenericDevices::Trigger::RELEASED;
            publicState();
            });
    }

    virtual ~Alarm() {}

    std::string type() { return std::string("alarm"); }

    std::vector<std::pair<std::string, std::string>> options() { return std::vector<std::pair<std::string, std::string>>(); }

    virtual void publicAll() {
        publicState();
    }

protected:

    boost::signals2::signal<void(void)> OnTriggered;
    boost::signals2::signal<void(void)> OnReleased;

    virtual void removeDevices() {
        trigger_->OnTriggered.disconnect_all_slots();
        trigger_.reset();

        FunctionalUnit::removeDevices();
    }

    void stateChannelCommandHandler(std::string command) {
        if (command == "?") publicState();
    }

    void publicState() { OnMessage(this, "state", (state_ == zigbee::GenericDevices::Trigger::TRIGGERED) ? "triggered" : "released"); }

    void attachTrigger(std::shared_ptr<zigbee::EndDevice> device) {
        trigger_ = std::dynamic_pointer_cast<zigbee::GenericDevices::Trigger>(device);

        trigger_->OnTriggered.connect([this](zigbee::GenericDevices::Trigger::State state) {
            (state == zigbee::GenericDevices::Trigger::TRIGGERED) ? OnTriggered() : OnReleased(); });
    }

    std::shared_ptr<zigbee::GenericDevices::Trigger> trigger_;
    zigbee::GenericDevices::Trigger::State state_ = zigbee::GenericDevices::Trigger::RELEASED;
};
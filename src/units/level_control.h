// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

class LevelControl : virtual public FunctionalUnit {
public:
    LevelControl() = delete;
    LevelControl(std::string id, zigbee::Coordinator* coordinator) : FunctionalUnit(id, coordinator) {
        BIND_ATTACH_HANDLER(zigbee::EndDevice::Type::LEVEL_CONTROL, LevelControl::attachLevel);
        BIND_CHANNEL_HANDLER("level", LevelControl::levelChannelCommandHandler);
    }

    virtual ~LevelControl() {}

    std::string type() { return std::string("level"); }

    std::vector<std::pair<std::string, std::string>> options() { return { {"level", std::to_string((unsigned)level_)} }; }

    virtual void publicAll() {
        publicLevel();
    }

protected:

    void levelChannelCommandHandler(std::string command) {
        if (level_control_)
            if (command == "?") publicLevel();
            else {
                try {
                    uint8_t level = std::clamp(std::stoi(command), 0, 255);
                    if (level_ != level) {
                        level_ = level;
                        level_control_->setLevel(level_);
                    } 
                }
                catch (std::exception error) {
                    OnError(this, error.what());  // post?
                }
            }
    }

    void publicLevel() { OnMessage(this, "level", std::to_string((unsigned)level_)); }

    void attachLevel(std::shared_ptr<zigbee::EndDevice> device) {
        level_control_ = std::dynamic_pointer_cast<zigbee::GenericDevices::LevelControledDevice>(device);
    }

    virtual void removeDevices() {
        level_control_.reset();
        FunctionalUnit::removeDevices();
    }

    std::shared_ptr<zigbee::GenericDevices::LevelControledDevice> level_control_;
    uint8_t level_ = 0;
};
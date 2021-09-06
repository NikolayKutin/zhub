// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

class Light : virtual public OnOff, virtual public LevelControl {
public:
    Light() = delete;
    Light(std::string id, zigbee::Coordinator* coordinator) : FunctionalUnit(id, coordinator), OnOff(id, coordinator), LevelControl(id, coordinator) {
        BIND_ATTACH_HANDLER(zigbee::EndDevice::Type::BUTTON, Light::attachButton);
    }

    virtual ~Light() {}

    std::string type() { return std::string("light"); }

    std::vector<std::pair<std::string, std::string>> options() { return std::vector<std::pair<std::string, std::string>>(); }

    virtual void publicAll() {
        OnOff::publicAll();
        LevelControl::publicAll();
    }

protected:

    void onClick() {
        if (switch_)
            try {
            if (state_ == OnOffSwitch::OFF) switch_->on();
            else switch_->off();
        }
        catch (std::runtime_error error) {
            OnError(this, error.what());  // post?
        }
    }

    void attachButton(std::shared_ptr<zigbee::EndDevice> device) {
        auto button = std::dynamic_pointer_cast<zigbee::GenericDevices::Button>(device);

        button->OnClick.connect(boost::bind(&Light::onClick, this));

        buttons_.push_back(button);
    }

    virtual void removeDevices() {
        for (auto button : buttons_) {
            button->OnClick.disconnect_all_slots();
            button->OnMultiClick.disconnect_all_slots();
        }
        buttons_.clear();

        OnOff::removeDevices();
        LevelControl::removeDevices();
    }

    std::vector<std::shared_ptr<zigbee::GenericDevices::Button>> buttons_;
};
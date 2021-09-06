// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "zigbee/zigbee.h"

#define BIND_ATTACH_HANDLER(type, member_func) attach_handlers_[type] = boost::bind(&member_func, this, boost::placeholders::_1)
#define BIND_CHANNEL_HANDLER(channel, member_func) channel_handlers_[channel] = boost::bind(&member_func, this, boost::placeholders::_1)

#define ADD_UNIT_CLASS(class_, identifier) FunctionalUnit::registerClass<class_>(identifier)

class FunctionalUnit : public utils::FactoryClass<FunctionalUnit, std::string, zigbee::Coordinator*> {
public:

    struct Descriptor {
        std::string id;
        std::string type;
        std::vector<zigbee::EndDevice::Descriptor> devices;
        std::vector<std::pair<std::string, std::string>> options; // <channel, command>
    };

    FunctionalUnit() = delete;
    FunctionalUnit(std::string id, zigbee::Coordinator* coordinator) : id_(id), coordinator_(coordinator) {
        BIND_CHANNEL_HANDLER("control", FunctionalUnit::controlChannelCommandHandler); }

    virtual ~FunctionalUnit() {}

    boost::signals2::signal<void(FunctionalUnit*, std::string, std::string)> OnMessage; // channel, message
    boost::signals2::signal<void(FunctionalUnit*, std::string)> OnError;
    boost::signals2::signal<void(FunctionalUnit*)> OnSettingsChange;

    static std::shared_ptr<FunctionalUnit> createUnit(Descriptor descriptor, zigbee::Coordinator* coordinator);

    bool attachDevice(std::shared_ptr<zigbee::EndDevice> device);
    bool attachDevice(zigbee::NetworkAddress network_address, zigbee::MacAddress ieee_address, std::string model);

    void executeCommand(std::string channel, std::string command);

    std::vector<std::shared_ptr<zigbee::EndDevice>> devices();
    std::string id() { return id_; }
    virtual std::string type() = 0;
    virtual std::vector<std::pair<std::string, std::string>> options() = 0; // < channel, command >

    Descriptor getDescriptor();

    virtual void publicAll() = 0;

protected:

    virtual void removeDevices();

    std::string id_;

    zigbee::Coordinator* coordinator_ = nullptr;
    std::vector <std::shared_ptr<zigbee::EndDevice>> devices_;

    std::map <zigbee::EndDevice::Type, std::function<void(std::shared_ptr<zigbee::EndDevice>)>> attach_handlers_;
    std::map <std::string, std::function<void(std::string)>> channel_handlers_;

private:

    bool permitJoin(const std::chrono::duration<int, std::ratio<1>> duration);
    void controlChannelCommandHandler(std::string command);

    boost::signals2::connection on_join_connection;

    bool initialized_ = false;
};
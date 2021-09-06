// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#include "functional_unit.h"

utils::GenericObjectFactory<std::string, FunctionalUnit, std::string, Coordinator*>
    utils::FactoryClass<FunctionalUnit, std::string, zigbee::Coordinator*>::factory_;

std::shared_ptr<FunctionalUnit> FunctionalUnit::createUnit(FunctionalUnit::Descriptor descriptor, zigbee::Coordinator* coordinator) {
    auto unit = createInstance(descriptor.type, descriptor.id, coordinator);

    for (auto device : descriptor.devices)
        unit->attachDevice(device.network_address, device.ieee_address, device.model);

    for (auto option : descriptor.options)
        unit->executeCommand(option.first, option.second);

    unit->initialized_ = true;

    return unit;
}

std::vector<std::shared_ptr<zigbee::EndDevice>>FunctionalUnit::devices() {
    std::vector<std::shared_ptr<zigbee::EndDevice>> connected_devices;

    for (auto device : devices_)
        if (*device) connected_devices.push_back(device); // Add NO-Leaved devices (operator bool)

    return connected_devices;
}

bool FunctionalUnit::attachDevice(std::shared_ptr<zigbee::EndDevice> device) {
    bool attached = false;

    for (zigbee::EndDevice::Type type : device->types())
        if (attach_handlers_.count(type)) {
            attached = true;
            attach_handlers_[type](device);
        }

    if (attached) {
        devices_.push_back(device);

        OnMessage(this, "control", "connected"); // post?

        if(initialized_)
            OnSettingsChange(this); // post?
    }

    return attached;
}

bool FunctionalUnit::attachDevice(zigbee::NetworkAddress network_address, zigbee::MacAddress ieee_address, std::string model) {
    return attachDevice(coordinator_->attachDevice(network_address, ieee_address, model));
}

void FunctionalUnit::executeCommand(std::string channel, std::string command) {
    if (channel_handlers_.count(channel))
        channel_handlers_[channel](command);
}

bool FunctionalUnit::permitJoin(const std::chrono::duration<int, std::ratio<1>> duration) {
    if (on_join_connection.connected())
        return false;

    on_join_connection = coordinator_->OnJoin.connect(boost::bind(&FunctionalUnit::attachDevice, this, boost::placeholders::_1));
    coordinator_->OnPermitJoinCompletion.connect(boost::bind(&boost::signals2::connection::disconnect, &on_join_connection));

    return coordinator_->permitJoin(duration);
}

void FunctionalUnit::controlChannelCommandHandler(std::string command) {
    if (!command.empty())
        try {
            if (command == "permitjoin")
                permitJoin(std::chrono::seconds(60)); // Todo time
            else if (command == "clear")
                removeDevices();
            else if (command == "options")
                for (auto option : options()) {
                    std::stringstream sstream;
                    sstream << "{\"" << option.first << "\": \"" << option.second << "\"}";

                    OnMessage(this, "control/options", sstream.str());
                }
        }
        catch (std::exception error) {
            OnError(this, error.what());
        }
}

FunctionalUnit::Descriptor FunctionalUnit::getDescriptor() {
    std::vector<zigbee::EndDevice::Descriptor> device_descriptors;
    for (auto device : devices())
        device_descriptors.push_back(device->getDescriptor());

    return { id_, type(), device_descriptors, options() };
}

void FunctionalUnit::removeDevices() {
    devices_.clear();
    OnSettingsChange(this); // post?
}
// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

class Coordinator;

class EndDevice : public Device, public utils::FactoryClass<zigbee::EndDevice, zigbee::NetworkAddress, zigbee::MacAddress, zigbee::Coordinator*> {
public:

    struct Descriptor {
        zigbee::NetworkAddress network_address;
        zigbee::MacAddress ieee_address;
        std::string model;
    };

    enum class Type { ON_OFF, LEVEL_CONTROL, BUTTON, TEMPERATURE_SENSOR, HUMIDITY_SENSOR, UNKNOWN, ILLUMINANCE_LEVEL_SENSOR, TRIGGER };

    static std::shared_ptr<EndDevice> createDevice(std::string identifier, Coordinator *coordinator, zigbee::NetworkAddress network_address, zigbee::MacAddress mac_address);
    static std::string getIdentifier(std::string model);
    template <class T> static void registerDeviceClass(std::string identifier, std::string model) { 
        EndDevice::registerClass<T>(identifier);
        EndDevice::model_to_identifier_table[model] = identifier;
    }

    EndDevice() = delete;
    EndDevice(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_address, Coordinator* coordinator);
    virtual ~EndDevice();

    boost::signals2::signal<void(void)> OnLeave;

    zigbee::NetworkAddress getNetworkAddress() { return network_address_; }
    zigbee::MacAddress getMacAddress() { return mac_address_; }

    virtual std::string model() const = 0;
    virtual std::vector<Type> types() const = 0;

    virtual void configure() {};

    operator bool() { return !leaved_; }

    Descriptor getDescriptor() const { return { network_address_, mac_address_, model() }; }

    static std::vector<std::string> getSupportedModels();

protected:

    zcl::Attribute readAttribute(uint8_t endpiont_number, zcl::Cluster cluster, uint16_t id);
    zcl::Status writeAttribute(uint8_t endpiont_number, zcl::Cluster cluster, zcl::Attribute attribute);
    zcl::Status sendCommand(uint8_t endpiont_number, zcl::Cluster cluster, uint8_t id, std::vector<uint8_t> payload = {});
    bool bind(uint8_t device_endpoint, uint8_t coordinator_endpoint, zcl::Cluster cluster);
    bool unbind(uint8_t device_endpoint, uint8_t coordinator_endpoint, zcl::Cluster cluster);
    bool configureReporting(uint8_t endpoint, zcl::Cluster cluster, uint16_t attribute_id, zcl::Attribute::DataType attribute_data_type,
        std::chrono::duration<uint16_t, std::ratio<1>> min_interval, std::chrono::duration<uint16_t, std::ratio<1>> max_interval, std::vector<uint8_t> reportable_change);

    zigbee::NetworkAddress network_address_;
    zigbee::MacAddress mac_address_;

    Coordinator *coordinator_ = nullptr;

    boost::signals2::connection on_attribute_report_connection_;
    boost::signals2::connection on_leave_connection_;

    virtual void onAttributeReport(uint8_t endpiont_number, zcl::Cluster cluster, zcl::Attribute attribute) = 0;

private:
    void onCoordinatorAttributeReport(zigbee::Endpoint endpoint, zcl::Cluster cluster, zcl::Attribute attribute);
    void onCoordinatorLeave(zigbee::NetworkAddress);

    bool leaved_ = false;

    static std::map<std::string, std::string> model_to_identifier_table;
};

class UnknownDevice : public EndDevice {
public:
    UnknownDevice() = delete;
    UnknownDevice(std::string identifier, zigbee::NetworkAddress network_address, zigbee::MacAddress mac_address, Coordinator *coordinator) : EndDevice(network_address, mac_address, coordinator), identifier_(identifier) {}

    ~UnknownDevice() {}

    std::string model() const { return (std::string("Unknown: ") + identifier_); }
    std::vector<Type> types() const { return { zigbee::EndDevice::Type::UNKNOWN }; }

private:
    void onAttributeReport(uint8_t endpiont_number, zcl::Cluster cluster, zcl::Attribute attribute) {}

    std::string identifier_;
};
// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

class Adapter : public Device {
public:
    boost::signals2::signal<void(zigbee::NetworkAddress, zigbee::MacAddress)> OnJoin;
    boost::signals2::signal<void(zigbee::NetworkAddress, zigbee::MacAddress)> OnLeave;
    boost::signals2::signal<void(zigbee::Message)> OnMessage;
    boost::signals2::signal<void(void)> OnDisconnect;

    virtual bool connect(std::string port, unsigned int baud_rate) = 0;
    virtual void disconnect() = 0;
    virtual bool reset() = 0;
    virtual bool ping() = 0;
    virtual bool startup() = 0;
    virtual bool registerEndpoint(zigbee::SimpleDescriptor endpointDescriptor) = 0;
    virtual bool permitJoin(const std::chrono::duration<int, std::ratio<1>> duration) = 0;
    virtual bool sendMessage(zigbee::Message message) = 0;
    virtual bool writeNetworkConfiguration(zigbee::NetworkConfiguration configuration) = 0;
    virtual std::vector<unsigned int> activeEndpoints(zigbee::NetworkAddress address) = 0;
    virtual zigbee::SimpleDescriptor simpleDescriptor(zigbee::NetworkAddress address, unsigned int endpointNumber) = 0;
    virtual std::vector<zigbee::NetworkAddress> associatedDevices() = 0;
    virtual std::optional<int> setTransmitPower(int dBm) = 0;
    virtual std::optional<std::pair<unsigned, unsigned>> version() = 0;
    virtual zigbee::NetworkConfiguration readNetworkConfiguration() = 0;

    virtual bool bind(zigbee::NetworkAddress device_network_address, zigbee::MacAddress source_mac_address, uint8_t source_endpoint,
        zigbee::MacAddress destination_mac_address, uint8_t destination_endpoint, zcl::Cluster cluster) = 0;
    virtual bool unbind(zigbee::NetworkAddress device_network_address, zigbee::MacAddress source_mac_address, uint8_t source_endpoint,
        zigbee::MacAddress destination_mac_address, uint8_t destination_endpoint, zcl::Cluster cluster) = 0;
};
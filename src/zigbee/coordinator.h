// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define GR_REP_CHANGE(T, x) (utils::to_uint8_vector<T>(T(x)))

class Coordinator : public Controller {
public:

    Coordinator() = delete;
    Coordinator(zigbee::Adapter *adapter, boost::asio::io_service *io_service);
    ~Coordinator();

    boost::signals2::signal<void(std::chrono::duration<int, std::ratio<1>>)> OnPermitJoin;
    boost::signals2::signal<void()> OnPermitJoinCompletion;

    boost::signals2::signal<void(std::shared_ptr<zigbee::EndDevice>)> OnJoin;
    boost::signals2::signal<void(zigbee::Endpoint, zcl::Cluster, zcl::Attribute)> OnAttributeReport;
    boost::signals2::signal<void(zigbee::NetworkAddress)> OnLeave;

    void start(zigbee::NetworkConfiguration configuration = Coordinator::default_configuration_);
    bool permitJoin(const std::chrono::duration<int, std::ratio<1>> duration);
    zcl::Attribute readAttribute(Endpoint endpoint, zcl::Cluster cluster, uint16_t id);
    zcl::Status writeAttribute(zigbee::Endpoint endpoint, zcl::Cluster cluster, zcl::Attribute attribute);
    zcl::Status sendCommand(Endpoint endpoint, zcl::Cluster cluster, uint8_t id, std::vector<uint8_t> payload = {}, zcl::FrameType frame_type = zcl::FrameType::SPECIFIC);
    std::shared_ptr<zigbee::EndDevice> attachDevice(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_adress, std::string model) { return joinDevice(network_address, mac_adress, EndDevice::getIdentifier(model)); }

    std::vector<std::shared_ptr<zigbee::EndDevice>> GenericDevices();

    bool bind(zigbee::Device* source_device, uint8_t source_endpoint, zigbee::Device* destination_device, uint8_t destination_endpoint, zcl::Cluster cluster);
    bool unbind(zigbee::Device* source_device, uint8_t source_endpoint, zigbee::Device* destination_device, uint8_t destination_endpoint, zcl::Cluster cluster);

    bool configureReporting(Endpoint endpoint, zcl::Cluster cluster, uint16_t attribute_id, zcl::Attribute::DataType attribute_data_type,
        std::chrono::duration<uint16_t, std::ratio<1>> min_interval, std::chrono::duration<uint16_t, std::ratio<1>> max_interval, std::vector<uint8_t> reportable_change);

    zigbee::NetworkAddress getNetworkAddress() { return adapter_->getNetworkAddress(); }
    zigbee::MacAddress getMacAddress() { return adapter_->getMacAddress(); }

    boost::asio::io_service* getIoService() { return io_service_; }
    zigbee::Adapter* getAdapter() { return adapter_; }

private:
    void onJoin(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_address);
    void onAttributeReport(zigbee::Endpoint endpoint, zcl::Cluster cluster, zcl::Attribute attribute);
    void onLeave(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_address);

    std::string getModelIdentifier(zigbee::NetworkAddress address);
    std::shared_ptr<zigbee::EndDevice> joinDevice(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_address, std::string identifier);

    boost::asio::io_service *io_service_ = nullptr;

    boost::signals2::connection on_join_connection_;
    boost::signals2::connection on_leave_connection_;
    boost::signals2::connection on_disconnect_connection_; // TODO: Add disconnect to EndDevice?

    std::map<zigbee::NetworkAddress, std::string> identifiers_; // Name of models assigned by manufacturers (transmitted over the network)
    std::map<zigbee::NetworkAddress, std::shared_ptr<zigbee::EndDevice>> devices_;
    utils::EventEmitter<std::string> identifier_emitter_;

    static const std::vector<zigbee::SimpleDescriptor> default_endpoints_;
    static const zigbee::NetworkConfiguration default_configuration_;                                                                                          // Disable.
};

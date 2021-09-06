// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

class Controller : public Device {
public:
    Controller() = delete;
    explicit Controller(zigbee::Adapter *adapter);
    ~Controller();

    boost::signals2::signal<void(std::string)> OnLog;
    boost::signals2::signal<void(std::runtime_error)> OnError;

    boost::signals2::signal<void(zigbee::Endpoint, zcl::Cluster, zcl::Attribute)> OnAttributeReport;

protected:

    void startNetwork(zigbee::NetworkConfiguration configuration);
    bool sendZclFrame(zigbee::Endpoint endpoint, zcl::Cluster cluster, zcl::Frame frame);
    zcl::Frame zclRequest(zigbee::Endpoint endpoint, zcl::Cluster cluster, zcl::Frame request);
    uint8_t genarateTransactionSequenceNumber() { static uint8_t transaction_sequence_number = 0; return transaction_sequence_number++; }

    zigbee::Adapter *adapter_ = nullptr;

private:
    void onMessage(zigbee::Message message);

    boost::signals2::connection on_message_connection_;

    utils::EventEmitter<zcl::Frame> zcl_frame_emitter_;
};
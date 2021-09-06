// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#include "zigbee.h"

using namespace zigbee;

Controller::Controller(zigbee::Adapter *adapter) : adapter_(adapter) {
    on_message_connection_ = adapter_->OnMessage.connect(boost::bind(&Controller::onMessage, this, boost::placeholders::_1));
}

Controller::~Controller() {
    on_message_connection_.disconnect();
}

void Controller::startNetwork(NetworkConfiguration configuration) {
    if(!adapter_->reset())
        throw(std::runtime_error("Adapter reseting hardware error"));

    OnLog("Reading network configuration");
    NetworkConfiguration currentConfiguration = adapter_->readNetworkConfiguration();

    if (currentConfiguration != configuration) { // To minimize NV memory write operations.
        OnLog("Writing correct network configuration");
        if (!adapter_->writeNetworkConfiguration(configuration))
            throw(std::runtime_error("Network configuration write fail"));
    }
        else {
            if (!adapter_->reset())
                throw(std::runtime_error("Adapter reseting hardware error"));
        }

    OnLog("Startup zigbee adapter");
    if (!adapter_->startup())
        throw(std::runtime_error("Adapter startup hardware error"));  
}

bool Controller::sendZclFrame(zigbee::Endpoint endpoint, zcl::Cluster cluster, zcl::Frame frame) {
    zigbee::Message message;

    message.cluster = cluster;
    message.source.number = 1; // TODO ?
    message.destination = endpoint;
    message.zcl_frame = frame;

    return adapter_->sendMessage(message);
}

void Controller::onMessage(zigbee::Message message) {
    if(static_cast<zcl::GlobalCommands>(message.zcl_frame.command) == zcl::GlobalCommands::REPORT_ATTRIBUTES) {
        try {
            for (auto attribute : zcl::Attribute::parseAttributesPayload(message.zcl_frame.payload, false)) 
                OnAttributeReport(message.source, message.cluster, attribute);
        }
        catch (std::exception error) {
            std::stringstream sstream;
            sstream << "Invalid received zcl payload (" << error.what() << ") from 0x" << std::hex << message.source.address;

            OnError(std::runtime_error(sstream.str()));
        } 
    }
    else
        zcl_frame_emitter_.emit(message.zcl_frame.transaction_sequence_number, message.zcl_frame);
}

zcl::Frame Controller::zclRequest(zigbee::Endpoint endpoint, zcl::Cluster cluster, zcl::Frame request) {
    zcl_frame_emitter_.clear(request.transaction_sequence_number);

    sendZclFrame(endpoint, cluster, request);
    
    std::optional<zcl::Frame> response = zcl_frame_emitter_.wait(request.transaction_sequence_number, 3s); // TODO: Zcl frame timeout.
    if(response)
        return *(response);
    else {
        std::stringstream sstream;
        sstream << "Zigbee end device 0x" << std::hex << endpoint.address << " not responsing";

        throw(std::runtime_error((sstream.str())));
    }
}
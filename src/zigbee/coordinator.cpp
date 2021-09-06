// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#include "zigbee.h"

using namespace zigbee;

const zigbee::NetworkConfiguration Coordinator::default_configuration_ = {  0xFFFF,                                                                                           // Pan ID.
                                                                            0xDDDDDDDDDDDDDDDD,                                                                               // Extended pan ID.
                                                                            zigbee::LogicalType::COORDINATOR,                                                                         // Logical type.
                                                                            {11},                                                                                             // RF channel list.
                                                                            {0x01, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0D, 0x0F, 0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0D}, // Precfg key.
                                                                            false
};

const std::vector<zigbee::SimpleDescriptor> Coordinator::default_endpoints_ = { {1,      // Enpoint number.
                                                                                0x0104, // Profile ID.
                                                                                0x05,   // Device ID.
                                                                                0,      // Device version.
                                                                                {},     // Input clusters list.
                                                                                {}} };   // Output clusters list.

Coordinator::Coordinator(zigbee::Adapter *adapter, boost::asio::io_service *io_service) : Controller(adapter), io_service_(io_service) {
    Controller::OnAttributeReport.connect(boost::bind(&Coordinator::onAttributeReport, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));

    on_join_connection_ = adapter_->OnJoin.connect(boost::bind(&Coordinator::onJoin, this, boost::placeholders::_1, boost::placeholders::_2));
    on_leave_connection_ = adapter_->OnLeave.connect(boost::bind(&Coordinator::onLeave, this, boost::placeholders::_1, boost::placeholders::_2));
}

Coordinator::~Coordinator() {
    on_join_connection_.disconnect(); // TODO: Unbind adapter
    on_leave_connection_.disconnect();
}

void Coordinator::start(zigbee::NetworkConfiguration configuration) {
    startNetwork(configuration);

    OnLog("Coordinator endpoints registration:");

    for (auto& endpoint_descriptor : default_endpoints_)
        if (adapter_->registerEndpoint(endpoint_descriptor))
            OnLog(std::move(toJson(endpoint_descriptor)));
        else {
            std::stringstream sstream;
            sstream << "Failure to register endpoint " << std::dec << endpoint_descriptor.endpoint_number << ", profile id: " << std::hex << endpoint_descriptor.profile_id;

            throw(std::runtime_error(std::move((sstream.str()))));
        }
}

zcl::Attribute Coordinator::readAttribute(zigbee::Endpoint endpoint, zcl::Cluster cluster, uint16_t id)
{
    zcl::Frame frame;
    {
        frame.frame_control.type = zcl::FrameType::GLOBAL;
        frame.frame_control.direction = zcl::FrameDirection::FROM_CLIENT_TO_SERVER;
        frame.frame_control.disable_default_response = false;
        frame.frame_control.manufacturer_specific = false;

        frame.command = zcl::GlobalCommands::READ_ATTRIBUTES;
        frame.transaction_sequence_number = genarateTransactionSequenceNumber();

        frame.payload.push_back(LOWBYTE(id));
        frame.payload.push_back(HIGHBYTE(id));
    }

    zcl::Frame read_attribute_response = zclRequest(endpoint, cluster, frame);

    if ((read_attribute_response.command != zcl::GlobalCommands::READ_ATTRIBUTES_RESPONSE) ||
        (read_attribute_response.transaction_sequence_number != frame.transaction_sequence_number)) {
        std::stringstream sstream;
        sstream << "Invalid zcl read attribute response (0x" << std::hex << endpoint.address << ", endpoint " << std::dec
            << static_cast<unsigned>(endpoint.number) << ", cluster: 0x" << std::hex << static_cast<unsigned>(cluster) << ")";

        throw(std::runtime_error((sstream.str())));
    }

    std::vector<zcl::Attribute> attributes = zcl::Attribute::parseAttributesPayload(read_attribute_response.payload, true);

    return (zcl::Attribute::parseAttributesPayload(read_attribute_response.payload, true))[0];
}

zcl::Status Coordinator::writeAttribute(zigbee::Endpoint endpoint, zcl::Cluster cluster, zcl::Attribute attribute) {
    zcl::Frame frame;
    {
        frame.frame_control.type = zcl::FrameType::GLOBAL;
        frame.frame_control.direction = zcl::FrameDirection::FROM_CLIENT_TO_SERVER;
        frame.frame_control.disable_default_response = false;
        frame.frame_control.manufacturer_specific = false;

        frame.command = zcl::GlobalCommands::WRITE_ATTRIBUTES;
        frame.transaction_sequence_number = genarateTransactionSequenceNumber();

        frame.payload.push_back(LOWBYTE(attribute.id));
        frame.payload.push_back(HIGHBYTE(attribute.id));

        frame.payload.push_back(static_cast<uint8_t>(attribute.data_type));

        std::vector<uint8_t> attribute_data = attribute.data();

        std::copy(attribute_data.begin(), attribute_data.end(), std::back_inserter(frame.payload));
    }

    zcl::Frame write_attribute_response = zclRequest(endpoint, cluster, frame);

    if ((write_attribute_response.command != zcl::GlobalCommands::WRITE_ATTRIBUTES_RESPONSE) ||
        (write_attribute_response.transaction_sequence_number != frame.transaction_sequence_number) ||
        (write_attribute_response.payload.size() < 3)) {
        /*(_UINT16(writeAttributeResponse->payload[1], writeAttributeResponse->payload[2]) == attribute.id)*/
        std::stringstream sstream;
        sstream << "Invalid zcl write attribute response (0x" << std::hex << endpoint.address << ", endpoint " << std::dec
            << static_cast<unsigned>(endpoint.number) << ", cluster: 0x" << std::hex << static_cast<unsigned>(cluster) << ")";

        throw(std::runtime_error((sstream.str())));
    }

    return static_cast<zcl::Status>(write_attribute_response.payload[0]);
}

zcl::Status Coordinator::sendCommand(zigbee::Endpoint endpoint, zcl::Cluster cluster, uint8_t id, std::vector<uint8_t> payload, zcl::FrameType frame_type) {
    zcl::Frame frame;

    frame.frame_control.type = frame_type;
    frame.frame_control.direction = zcl::FrameDirection::FROM_CLIENT_TO_SERVER;
    frame.frame_control.disable_default_response = false;
    frame.frame_control.manufacturer_specific = false;

    frame.command = id;
    frame.transaction_sequence_number = genarateTransactionSequenceNumber();
    frame.payload = payload;

    zcl::Frame default_response = zclRequest(endpoint, cluster, frame);

    if ((default_response.command != zcl::GlobalCommands::DEFAULT_RESPONSE) ||
        (default_response.transaction_sequence_number != frame.transaction_sequence_number) ||
        (default_response.payload[0] != frame.command)) {
        std::stringstream sstream;
        sstream << "Invalid zcl command response (0x" << std::hex << endpoint.address << ", endpoint " << std::dec
            << static_cast<unsigned>(endpoint.number) << ", cluster: 0x" << std::hex << static_cast<unsigned>(cluster) << ")";

        throw(std::runtime_error((sstream.str())));
    }

    return static_cast<zcl::Status>(default_response.payload[1]);
}

std::string Coordinator::getModelIdentifier(zigbee::NetworkAddress address) {
    if (identifiers_.count(address))
        return identifiers_[address];
    else {
        std::thread identifier_handler([this, address]{
            try {
                zcl::Attribute identifier_response = this->readAttribute({address, 1}, zcl::Cluster::BASIC, zcl::Attributes::Basic::MODEL_IDENTIFIER);

                std::string identifier = std::any_cast<std::string>(identifier_response.value);

                this->identifier_emitter_.emit(address, identifier);
            }
            catch(std::exception) { 
                if (!identifiers_.count(address)) {
                    std::stringstream sstream;
                    sstream << "Identifier attribute reading error: 0x" << std::hex << address;
                    io_service_->post(boost::bind(boost::ref(OnError), std::runtime_error(sstream.str())));
                }
            }
        });
        identifier_handler.detach();

        std::optional<std::string> identifier = identifier_emitter_.wait(address, 10s); // TODO: timeout
        if(identifier)
            return *identifier;
        else {
            std::stringstream sstream;
            sstream << "Identifier attribute time out: 0x" << std::hex << address;
            io_service_->post(boost::bind(boost::ref(OnError), std::runtime_error(sstream.str())));

            return std::string();
        }
    }
}

void Coordinator::onJoin(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_address) {
    io_service_->post([this, network_address, mac_address]() {
        std::string identifier = this->getModelIdentifier(network_address);
        if (identifier.empty() ||
            ((this->identifiers_.count(network_address)) && (this->identifiers_[network_address] == identifier)))
            return;

        std::shared_ptr<zigbee::EndDevice> joined_device = joinDevice(network_address, mac_address, identifier);

        joined_device->configure();

        OnJoin(joined_device);
    });
}

void Coordinator::onAttributeReport(zigbee::Endpoint endpoint, zcl::Cluster cluster, zcl::Attribute attribute) {
    if (cluster == zcl::Cluster::BASIC && attribute.id == zcl::Attributes::Basic::MODEL_IDENTIFIER) {
        try {
            identifier_emitter_.emit(endpoint.address, std::any_cast<std::string>(attribute.value));
        }
        catch (std::bad_any_cast) {
            std::stringstream sstream;
            sstream << "Invalid identifier attribute from 0x" << std::hex << endpoint.address;
            io_service_->post(boost::bind(boost::ref(OnError), std::runtime_error(sstream.str())));
        }
    }

    io_service_->post(boost::bind(boost::ref(OnAttributeReport), endpoint, cluster, attribute));
}

void Coordinator::onLeave(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_address) { // TODO: disconnect event
    if (identifiers_.count(network_address))
        identifiers_.erase(network_address);

    if (devices_.count(network_address))
        devices_.erase(network_address);

    io_service_->post(boost::bind(boost::ref(OnLeave), network_address));    
}

std::shared_ptr<zigbee::EndDevice> Coordinator::joinDevice(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_address, std::string identifier) {
    std::shared_ptr<zigbee::EndDevice> joined_device = zigbee::EndDevice::createDevice(identifier, this, network_address, mac_address);

    if (!joined_device)
        joined_device = std::make_shared<zigbee::UnknownDevice>(identifier, network_address, mac_address, this);

    identifiers_[network_address] = identifier;
    devices_[joined_device->getNetworkAddress()] = joined_device;

    return joined_device;
}

std::vector<std::shared_ptr<zigbee::EndDevice>> Coordinator::GenericDevices() {
    std::vector<std::shared_ptr<zigbee::EndDevice>> result;

    for(auto device_pair : devices_)
        result.push_back(device_pair.second);

    return result;    
}

bool Coordinator::configureReporting(Endpoint endpoint, zcl::Cluster cluster, uint16_t attribute_id, zcl::Attribute::DataType attribute_data_type,
    std::chrono::duration<uint16_t, std::ratio<1>> min_interval, std::chrono::duration<uint16_t, std::ratio<1>> max_interval, std::vector<uint8_t> reportable_change) {
    return true; // TODO:
}

bool Coordinator::permitJoin(const std::chrono::duration<int, std::ratio<1>> duration) {
    if (!adapter_->permitJoin(duration))  return false; 
        
    io_service_->post(boost::bind(boost::ref(OnPermitJoin), duration));

    static boost::asio::deadline_timer timer(*io_service_);
    timer.expires_from_now(boost::posix_time::seconds(duration.count()));
    timer.async_wait([this](const boost::system::error_code& ec) { if (!ec) OnPermitJoinCompletion(); });

    return true;
}

bool Coordinator::bind(zigbee::Device* source_device, uint8_t source_endpoint, zigbee::Device* destination_device, uint8_t destination_endpoint, zcl::Cluster cluster) {
    std::stringstream sstream;
    sstream << "Binding: [0x" << std::hex << source_device->getNetworkAddress() << ", 0x" << source_device->getMacAddress() << "ep" <<
        std::dec << static_cast<unsigned>(source_endpoint) << "] -> [0x" << std::hex << destination_device->getNetworkAddress() << ", 0x" <<
        destination_device->getMacAddress() << "ep" << std::dec << static_cast<unsigned>(destination_endpoint) << "] cluster: 0x" << std::hex << static_cast<unsigned>(cluster);
    io_service_->post(boost::bind(boost::ref(OnLog), sstream.str()));

    return adapter_->bind(source_device->getNetworkAddress(), source_device->getMacAddress(), source_endpoint, destination_device->getMacAddress(), destination_endpoint, cluster);
}

bool Coordinator::unbind(zigbee::Device* source_device, uint8_t source_endpoint, zigbee::Device* destination_device, uint8_t destination_endpoint, zcl::Cluster cluster) {
    return adapter_->unbind(source_device->getNetworkAddress(), source_device->getMacAddress(), source_endpoint, destination_device->getMacAddress(), destination_endpoint, cluster);
}
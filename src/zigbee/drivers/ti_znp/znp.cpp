// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#include "../../zigbee.h"

using namespace zigbee;
using namespace TiUnp;
using namespace TiZnp;

Znp::Znp(boost::asio::io_service &ios) : io_service_(&ios), unpi_(ios), current_state_(ZdoState::NOT_STARTED) {
    unpi_.OnCommand.connect(boost::bind(&Znp::handleCommand, this, boost::placeholders::_1));
    unpi_.OnDisconnect.connect(boost::bind(&Znp::onDisconnect, this));
}

Znp::~Znp() { unpi_.disconnect(); }

bool Znp::reset(ResetType reset_type, bool clear_network_state, bool clear_config) {
    uint8_t startup_options = static_cast<uint8_t>(clear_network_state << 1) + static_cast<uint8_t>(clear_network_state);

    writeNv(NvItems::STARTUP_OPTION, std::vector<uint8_t> { startup_options });

    Command reset_request(SYS_RESET_REQ);
    reset_request.payload(0) = static_cast<uint8_t>(reset_type);

    std::optional<Command> reset_response = waitResponse(reset_request, SYS_RESET_IND, RESET_TIMEOUT);

    /*
    int reason = static_cast<int>(resetResponse.payload(1);); // Power-up 0x00, External 0x01, Watch-dog 0x02
    int transportRevision = static_cast<int>(resetResponse.payload(2);); // Transport protocol revision.
    int product = static_cast<int>(resetResponse.payload(2);); // ID
    int majorRelease = static_cast<int>(resetResponse.payload(3);); // Major release number.
    int minorRelease = static_cast<int>(resetResponse.payload(4);); // Minor release number.
    int hardwareRevision = static_cast<int>(resetResponse.payload(5);); //Hardware revision number.
    */

    if (reset_response) {
        version_.first = reset_response->payload(3);
        version_.second = reset_response->payload(4);

        return true;
    }
    else
        return false;
}

bool Znp::ping() { return(syncRequest(Command(SYS_PING)).has_value()); }

bool Znp::startup(std::chrono::duration<int, std::milli> delay) {
    Command startup_request(ZDO_STARTUP_FROM_APP);
    startup_request.payload(0) = LOWBYTE(delay.count());
    startup_request.payload(1) = HIGHBYTE(delay.count());

    std::optional<Command> startup_response = syncRequest(startup_request);

    enum class SturtupStatus { RESTORED_STATE = 0, NEW_STATE = 1, NOT_STARTED = 2 }; // 0 - Restored network state, 1 - New network state, 2 - Leave and not Started.

    if ((!startup_response) || (static_cast<SturtupStatus>(startup_response->payload(0)) == SturtupStatus::NOT_STARTED))
        return false;

    std::optional<Command> device_info_response = syncRequest(Command(UTIL_GET_DEVICE_INFO));

    if (verifySyncResponse(device_info_response) && (static_cast<Status>(device_info_response->payload(0)) == Status::SUCCESS)) {
        network_address_ = _UINT16(device_info_response->payload(9), device_info_response->payload(10));
        mac_address_ = *((zigbee::MacAddress*)(device_info_response->data() + 1));
    }

    return true;
}

bool Znp::registerEndpoint(zigbee::SimpleDescriptor endpoint_descriptor) {
    if (endpoint_descriptor.endpoint_number > 240) return false;

    Command register_ep_request(AF_REGISTER);

    {
        size_t i = 0;

        register_ep_request.payload(i++) = static_cast<uint8_t>(endpoint_descriptor.endpoint_number);

        register_ep_request.payload(i++) = LOWBYTE(endpoint_descriptor.profile_id);
        register_ep_request.payload(i++) = HIGHBYTE(endpoint_descriptor.profile_id);

        register_ep_request.payload(i++) = LOWBYTE(endpoint_descriptor.device_id);
        register_ep_request.payload(i++) = HIGHBYTE(endpoint_descriptor.device_id);

        register_ep_request.payload(i++) = static_cast<uint8_t>(endpoint_descriptor.device_version);

        register_ep_request.payload(i++) = 0; // 0x00 - No latency*, 0x01 - fast beacons, 0x02 - slow beacons.     

        register_ep_request.payload(i++) = static_cast<uint8_t>(endpoint_descriptor.input_clusters.size());
        for (auto cluster : endpoint_descriptor.input_clusters)
            register_ep_request.payload(i++) = static_cast<uint8_t>(cluster);

        register_ep_request.payload(i++) = static_cast<uint8_t>(endpoint_descriptor.output_clusters.size());
        for (auto cluster : endpoint_descriptor.output_clusters)
            register_ep_request.payload(i++) = static_cast<uint8_t>(cluster);
    }

    return verifySyncResponse(syncRequest(register_ep_request));
}

bool Znp::permitJoin(const std::chrono::duration<int, std::ratio<1>> duration) {
    Command  permit_join_request(ZDO_MGMT_PERMIT_JOIN_REQ);

    permit_join_request.payload(0) = 0x0F; // Destination address type : 0x02 � Address 16 bit, 0x0F � Broadcast.

    permit_join_request.payload(1) = 0xFC; // Specifies the network address of the destination device whose Permit Join information is to be modified.
    permit_join_request.payload(2) = 0xFF; // (address || 0xFFFC)

    permit_join_request.payload(3) = std::min(duration.count(), 254); // Maximum duration.

    permit_join_request.payload(4) = 0x00; // Trust Center Significance (0).

    std::optional<Command> permit_join_ind = asyncRequest(permit_join_request, ZDO_MGMT_PERMIT_JOIN_RSP);

    return ((permit_join_ind) && (static_cast<Status>(permit_join_ind->payload(2)) == Status::SUCCESS)); // TODO: Error handling?
}

std::vector<unsigned int> Znp::activeEndpoints(zigbee::NetworkAddress address) {
    std::vector<unsigned int> endpoints;

    Command active_endpoints_request(ZDO_ACTIVE_EP_REQ);

    active_endpoints_request.payload(0) = LOWBYTE(address); // Specifies NWK address of the device generating the inquiry.
    active_endpoints_request.payload(1) = HIGHBYTE(address);

    active_endpoints_request.payload(2) = LOWBYTE(address); // Specifies NWK address of the destination device being queried.
    active_endpoints_request.payload(3) = HIGHBYTE(address);

    std::optional<Command> active_endpoints_async_response = asyncRequest(active_endpoints_request, ZDO_ACTIVE_EP_RSP);

    if ((active_endpoints_async_response) && (static_cast<Status>(active_endpoints_async_response->payload(2)) == Status::SUCCESS)) { // TODO: Error handling?
        for (int i = 0; i < static_cast<int>(active_endpoints_async_response->payload(5)); i++) // Number of active endpoint in the list
            endpoints.push_back(static_cast<unsigned int>(active_endpoints_async_response->payload(6 + i)));
    }

    return endpoints;
}

zigbee::SimpleDescriptor Znp::simpleDescriptor(zigbee::NetworkAddress address, unsigned int endpoint_number) {
    assert(endpoint_number <= 240);

    zigbee::SimpleDescriptor descriptor;

    Command simple_descriptor_request(ZDO_SIMPLE_DESC_REQ);

    simple_descriptor_request.payload(0) = LOWBYTE(address); // Specifies NWK address of the device generating the inquiry.
    simple_descriptor_request.payload(1) = HIGHBYTE(address);

    simple_descriptor_request.payload(2) = LOWBYTE(address); // Specifies NWK address of the destination device being queried.
    simple_descriptor_request.payload(3) = HIGHBYTE(address);

    simple_descriptor_request.payload(4) = static_cast<uint8_t>(endpoint_number);

    std::optional<Command> simple_descriptor_async_response = asyncRequest(simple_descriptor_request, ZDO_SIMPLE_DESC_RSP);

    if ((simple_descriptor_async_response) && (static_cast<Status>(simple_descriptor_async_response->payload(2)) == Status::SUCCESS)) { // TODO: Error handling?
        descriptor.endpoint_number = static_cast<unsigned int>(simple_descriptor_async_response->payload(6));

        descriptor.profile_id = _UINT16(simple_descriptor_async_response->payload(7), simple_descriptor_async_response->payload(8));

        descriptor.device_id = _UINT16(simple_descriptor_async_response->payload(9), simple_descriptor_async_response->payload(10));

        descriptor.device_version = static_cast<unsigned int>(simple_descriptor_async_response->payload(11));

        {
            size_t i = 12; // Index of number of input clusters/

            int input_clusters_number = static_cast<int>(simple_descriptor_async_response->payload(i++));
            while (input_clusters_number--)
                descriptor.input_clusters.push_back(static_cast<unsigned int>(_UINT16(   simple_descriptor_async_response->payload(i++),
                                                                                        simple_descriptor_async_response->payload(i++)))); // List of input cluster Id�s supported.

            int output_clusters_number = static_cast<int>(simple_descriptor_async_response->payload(i++));
            while (output_clusters_number--)
                descriptor.output_clusters.push_back(static_cast<unsigned int>(_UINT16(  simple_descriptor_async_response->payload(i++),
                                                                                        simple_descriptor_async_response->payload(i++)))); // List of output cluster Id�s supported.
        }
    }

    return descriptor;
}

std::optional<std::vector<uint8_t>> Znp::readNv(NvItems item) {
    std::vector<uint8_t> value;

    Command read_nv_request(SYS_OSAL_NV_READ);

    read_nv_request.payload(0) = LOWBYTE(item); // The Id of the NV item.
    read_nv_request.payload(1) = HIGHBYTE(item);

    read_nv_request.payload(2) = 0; // Number of bytes offset from the beginning or the NV value.

    std::optional<Command> read_nv_response = syncRequest(read_nv_request);

    if (verifySyncResponse(read_nv_response)) {
        size_t length = static_cast<size_t>(read_nv_response->payload(1));
        std::copy_n(read_nv_response->data() + 2, length, back_inserter(value));

        return value;
    }

    return std::nullopt;
}

bool Znp::writeNv(NvItems item, std::vector<uint8_t> item_data) {
    assert(item_data.size() <= 128);

    Command write_nv_request(SYS_OSAL_NV_WRITE, item_data.size() + 4);

    {
        write_nv_request.payload(0) = LOWBYTE(item); // The Id of the NV item.
        write_nv_request.payload(1) = HIGHBYTE(item);

        write_nv_request.payload(2) = 0; // Number of bytes offset from the beginning or the NV value.

        write_nv_request.payload(3) = static_cast<uint8_t>(item_data.size());

        std::copy(item_data.begin(), item_data.end(), write_nv_request.data() + 4);
    }

    return verifySyncResponse(syncRequest(write_nv_request));
}

bool Znp::initNv(NvItems item, uint16_t length, std::vector<uint8_t> item_data) {
    assert(item_data.size() <= 245);

    Command init_nv_request(SYS_OSAL_NV_ITEM_INIT);

    init_nv_request.payload(0) = LOWBYTE(item); // The Id of the NV item.
    init_nv_request.payload(1) = HIGHBYTE(item);

    init_nv_request.payload(2) = LOWBYTE(length); // Number of bytes in the NV item.
    init_nv_request.payload(3) = HIGHBYTE(length);

    init_nv_request.payload(4) = static_cast<uint8_t>(item_data.size()); // Number of bytes in the initialization data.

    std::copy(item_data.begin(), item_data.end(), init_nv_request.data() + 5);

    std::optional<Command> init_nv_response = syncRequest(init_nv_request);

    return (init_nv_response && (init_nv_response->payload(0) != 0x0A));// 0x00 = Item already exists, no action taken
                                                                        // 0x09 = Success, item created and initialized
                                                                        // 0x0A = Initialization failed, item not created
                                                                        // TODO: Error handling?
}

std::vector<zigbee::NetworkAddress> Znp::associatedDevices() {
    std::vector<zigbee::NetworkAddress> GenericDevices;

    std::optional<Command> device_info_response = syncRequest(Command(UTIL_GET_DEVICE_INFO));

    if (verifySyncResponse(device_info_response)) {
        unsigned associated_devices_number = device_info_response->payload(13);

        zigbee::NetworkAddress address;
        for (unsigned int i = 0; i < associated_devices_number; i++) {
            address = _UINT16(  device_info_response->payload(14 + (i * sizeof(address))), 
                                device_info_response->payload(15 + (i * sizeof(address))));

            GenericDevices.push_back(address);
        }
    }

    return GenericDevices;
}

std::optional <int> Znp::setTransmitPower(int power) { // Returned actual TX power (dBm). Range: -22 ... 3 dBm.
    Command tx_power_request(SYS_SET_TX_POWER);

    tx_power_request.payload(0) = LOWBYTE(power);

    std::optional<Command> tx_power_response = syncRequest(tx_power_request);

    if (tx_power_response)
        return tx_power_response->payload(0);
    else
        return std::nullopt;
}

std::optional<std::pair<unsigned, unsigned>> Znp::version() {
    if (version_ == std::pair<unsigned, unsigned> { 0, 0 }) {
        std::optional<Command> version_response = syncRequest(Command(SYS_VERSION));

        if (version_response) {
            version_.first = version_response->payload(2);
            version_.second = version_response->payload(3);
        }
        else
            return std::nullopt;
    }
    
    return version_;
}

void Znp::handleCommand(Command command) {
    switch (command.id()) {

    case AF_INCOMING_MSG: {
        try {
            zigbee::Message message;

            message.cluster = static_cast<zcl::Cluster>(_UINT16(command.payload(2), command.payload(3)));
            message.source.address = _UINT16(command.payload(4), command.payload(5));
            message.destination.address = network_address_;
            message.source.number = command.payload(6);
            message.destination.number = command.payload(7);

            //uint8_t linkQuality = command.payload(9);
            size_t length = static_cast<size_t>(command.payload(16));

            message.zcl_frame = parseZclData(std::vector<uint8_t>(&command.payload(17), &command.payload(17 + length)));
            OnMessage(message);
        }
        catch (std::out_of_range) { std::cerr << "bad Zcl data" << std::endl; } // TODO: Error handling (Receiver thread).

        break;
    }

    case ZDO_STATE_CHANGE_IND: {
        current_state_ = static_cast<ZdoState>(command.payload(0));
        break;
    }

    case ZDO_TC_DEV_IND: {
        zigbee::NetworkAddress networkAddress = _UINT16(command.payload(0), command.payload(1));
        zigbee::MacAddress macAddress = *(reinterpret_cast<zigbee::MacAddress*>(&command.payload(2)));

        OnJoin(networkAddress, macAddress);

        break;
    }

    /*
    case ZDO_END_DEVICE_ANNCE_IND: {
        break;
    }
    */

    case ZDO_LEAVE_IND: {
        zigbee::NetworkAddress network_address = _UINT16(command.payload(0), command.payload(1));
        zigbee::MacAddress mac_address = *(reinterpret_cast<zigbee::MacAddress*>(&command.payload(2)));

        OnLeave(network_address, mac_address);

        break;
    }

    default: command_emitter_.emit(command.id(), command);
    }
}

zcl::Frame Znp::parseZclData(std::vector<uint8_t> data) { // If there is an error in the data, an exception std::out_of_range may be thrown.
    zcl::Frame zcl_frame;

    zcl_frame.frame_control.type = static_cast<zcl::FrameType>(data.at(0) & 0b00000011);
    zcl_frame.frame_control.manufacturer_specific = static_cast<bool>(data.at(0) & 0b00000100);
    zcl_frame.frame_control.direction = static_cast<zcl::FrameDirection>(data.at(0) & 0b00001000);
    zcl_frame.frame_control.disable_default_response = static_cast<bool>(data.at(0) & 0b00010000);

    if (zcl_frame.frame_control.manufacturer_specific)
        zcl_frame.manufacturer_code = _UINT16(data.at(1), data.at(2));

    size_t i = (zcl_frame.frame_control.manufacturer_specific) ? 3 : 1;

    zcl_frame.transaction_sequence_number = data.at(i++);
    zcl_frame.command = data.at(i++);

    std::copy_n(data.data() + i, data.size() - i, std::back_inserter(zcl_frame.payload));

    return zcl_frame;
}

bool Znp::sendMessage(zigbee::Message message) {
    uint8_t transaction_number = genarateTransactionNumber();

    if ((message.zcl_frame.payload.size() + (message.zcl_frame.frame_control.manufacturer_specific * sizeof(message.zcl_frame.manufacturer_code)) + 3) > 128)
        return false;

    Command af_data_request(AF_DATA_REQUEST);

    {
        af_data_request.payload(0) = LOWBYTE(message.destination.address);
        af_data_request.payload(1) = HIGHBYTE(message.destination.address);

        af_data_request.payload(2) = message.destination.number;
        af_data_request.payload(3) = message.source.number;

        af_data_request.payload(4) = LOWBYTE(message.cluster);
        af_data_request.payload(5) = HIGHBYTE(message.cluster);

        af_data_request.payload(6) = transaction_number;

        af_data_request.payload(7) = 0; // Options.

        af_data_request.payload(8) = DEFAULT_RADIUS;

        af_data_request.payload(10) = (static_cast<uint8_t>(message.zcl_frame.frame_control.type) & 0b00000011) +
                                    (static_cast<uint8_t>(message.zcl_frame.frame_control.manufacturer_specific) << 2) +
                                    (static_cast<uint8_t>(message.zcl_frame.frame_control.direction)) +
                                    (static_cast<uint8_t>(message.zcl_frame.frame_control.disable_default_response) << 4);

        uint8_t i = 11;

        if (message.zcl_frame.frame_control.manufacturer_specific) {
            af_data_request.payload(i++) = LOWBYTE(message.zcl_frame.manufacturer_code);
            af_data_request.payload(i++) = HIGHBYTE(message.zcl_frame.manufacturer_code);
        }

        af_data_request.payload(i++) = message.zcl_frame.transaction_sequence_number;
        af_data_request.payload(i++) = message.zcl_frame.command;

        for (auto byte : message.zcl_frame.payload)
            af_data_request.payload(i++) = byte;

        af_data_request.payload(9) = i; // data length.
    }

    std::optional<Command> af_data_confirm = asyncRequest(af_data_request, AF_DATA_CONFIRM , 2s); // TODO: timeout.

    return  ((af_data_confirm)
            && (af_data_confirm->payload(2) == transaction_number) 
            && (static_cast<TiZnp::Status>(af_data_confirm->payload(0)) == Status::SUCCESS)); // TODO: Error handing.
}

zigbee::NetworkConfiguration Znp::readNetworkConfiguration() {
    std::optional<std::vector<uint8_t>> item_data;
    zigbee::NetworkConfiguration configuration;

    item_data = readNv(NvItems::PAN_ID);
    if (item_data && item_data->size() == 2)
        configuration.pan_id = _UINT16((*item_data)[0], (*item_data)[1]);

    item_data = readNv(NvItems::EXTENDED_PAN_ID);
    if (item_data && item_data->size() == 8)
        configuration.extended_pan_id= *(reinterpret_cast<uint64_t*>(item_data->data()));

    item_data = readNv(NvItems::LOGICAL_TYPE);
    if (item_data && item_data->size() == 4)
        configuration.logical_type = static_cast<zigbee::LogicalType>((*item_data)[0]); // TODO: Logical type

    /*
    value = readNv(NvItems::ZDO_DIRECT_CB);
    if (value.size())
        configuration.zdoDirectCallback = value[0];
    */

    item_data = readNv(NvItems::PRECFG_KEYS_ENABLE);
    if (item_data && item_data->size() == 1)
        configuration.precfg_key_enable = (*item_data)[0];

    item_data = readNv(NvItems::PRECFG_KEY);
    if (item_data && item_data->size() == 16)
        std::copy_n(item_data->begin(), 16, configuration.precfg_key.begin());

    item_data = readNv(NvItems::CHANNEL_LIST);
    if (item_data && item_data->size() == 4) {
        uint32_t channelBitMask = *(reinterpret_cast<uint32_t*>(item_data->data()));

        for (unsigned int i = 0; i < 32; i++)
            if (channelBitMask & (1 << i))
                configuration.channels.push_back(i);
    }

    return configuration;
}

bool Znp::writeNetworkConfiguration(zigbee::NetworkConfiguration configuration) {
    if (!writeNv(NvItems::PAN_ID, utils::to_uint8_vector<uint16_t>(configuration.pan_id))) return false;

    if (!writeNv(NvItems::EXTENDED_PAN_ID, utils::to_uint8_vector<uint64_t>(configuration.extended_pan_id))) return false;

    if (!writeNv(NvItems::LOGICAL_TYPE, std::vector<uint8_t> { static_cast<uint8_t>(configuration.logical_type)})) return false; // TODO: Logical type

    if (!writeNv(NvItems::PRECFG_KEYS_ENABLE, std::vector<uint8_t> { static_cast<uint8_t>(configuration.precfg_key_enable)})) return false;

    if (!writeNv(NvItems::PRECFG_KEY, std::vector<uint8_t>(std::begin(configuration.precfg_key), std::end(configuration.precfg_key)))) return false;

    {
        uint32_t channelBitMask = 0;
        for (auto channel : configuration.channels)
            channelBitMask |= (1 << channel);

        if (!writeNv(NvItems::CHANNEL_LIST, utils::to_uint8_vector<uint32_t>(channelBitMask))) return false;
    }

    if (!writeNv(NvItems::ZDO_DIRECT_CB, std::vector<uint8_t> { 1 })) return false;

    if (!initNv(NvItems::ZNP_HAS_CONFIGURED, 1, std::vector<uint8_t> { 0 })) return false;

    if (!writeNv(NvItems::ZNP_HAS_CONFIGURED, std::vector<uint8_t> { 0x55 })) return false;

    return true;
}

std::optional<Command> Znp::waitResponse(Command request, uint16_t id, std::chrono::duration<int, std::milli> timeout) {
    command_emitter_.clear(id);

    if (unpi_.sendCommand(request))
        return command_emitter_.wait(id, timeout);
    else
        return std::nullopt; 
}

std::optional<Command> Znp::syncRequest(Command request, std::chrono::duration<int, std::milli> timeout) {
    assert(request.type() == Command::Type::SREQ);

    return waitResponse(request, (request.id() | 0b0100000000000000), timeout); // SREQ(0b001) -> SRES(0b011)
}

std::optional<Command> Znp::asyncRequest(Command& request, uint16_t async_response_id, std::chrono::duration<int, std::milli> timeout) {
    command_emitter_.clear(async_response_id);

    if (verifySyncResponse(syncRequest(request)))
        return command_emitter_.wait(async_response_id, timeout);
    else
        return std::nullopt;  
}

bool Znp::verifySyncResponse(std::optional<Command> response) { return ((response) && (response->payload(0) == 0x00)); } // TODO: SUCCESS

bool  Znp::bind(zigbee::NetworkAddress device_network_address, zigbee::MacAddress source_mac_address, uint8_t source_endpoint, zigbee::MacAddress destination_mac_address, uint8_t destination_endpoint, zcl::Cluster cluster) {
    Command bind_request(ZDO_BIND_REQ);
    {
        size_t i = 0;

        bind_request.payload(i++) = LOWBYTE(device_network_address);
        bind_request.payload(i++) = HIGHBYTE(device_network_address);

        for (auto byte : utils::to_uint8_vector<zigbee::MacAddress>(source_mac_address))
            bind_request.payload(i++) = byte;

        bind_request.payload(i++) = source_endpoint;

        bind_request.payload(i++) = LOWBYTE(cluster);
        bind_request.payload(i++) = HIGHBYTE(cluster);

        bind_request.payload(i++) = 0x03; // ADDRESS_64_BIT

        for (auto byte : utils::to_uint8_vector<zigbee::MacAddress>(destination_mac_address))
            bind_request.payload(i++) = byte;

        bind_request.payload(i++) = destination_endpoint;
    }

    std::optional<Command> bind_response = asyncRequest(bind_request, ZDO_BIND_RSP, 1s); // TODO: timeout.

    return  ((bind_response)
        //&& (static_cast<zigbee::NetworkAddress>(_UINT16(bind_response->payload(0), bind_response->payload(1))) == device_network_address)
        && (static_cast<TiZnp::Status>(bind_response->payload(2)) == Status::SUCCESS)); // TODO: Error handing.
}

bool  Znp::unbind(zigbee::NetworkAddress device_network_address, zigbee::MacAddress source_mac_address, uint8_t source_endpoint, zigbee::MacAddress destination_mac_address, uint8_t destination_endpoint, zcl::Cluster cluster) {
    Command bind_request(ZDO_UNBIND_REQ);
    {
        size_t i = 0;

        bind_request.payload(i++) = LOWBYTE(device_network_address);
        bind_request.payload(i++) = HIGHBYTE(device_network_address);

        for (auto byte : utils::to_uint8_vector<zigbee::MacAddress>(source_mac_address))
            bind_request.payload(i++) = byte;

        bind_request.payload(i++) = source_endpoint;

        bind_request.payload(i++) = LOWBYTE(cluster);
        bind_request.payload(i++) = HIGHBYTE(cluster);

        bind_request.payload(i++) = 0x03; // ADDRESS_64_BIT

        for (auto byte : utils::to_uint8_vector<zigbee::MacAddress>(destination_mac_address))
            bind_request.payload(i++) = byte;

        bind_request.payload(i++) = destination_endpoint;
    }

    std::optional<Command> bind_response = asyncRequest(bind_request, ZDO_UNBIND_RSP, 1s); // TODO: timeout.

    return  ((bind_response)
        //&& (static_cast<zigbee::NetworkAddress>(_UINT16(bind_response->payload(0), bind_response->payload(1))) == device_network_address)
        && (static_cast<TiZnp::Status>(bind_response->payload(2)) == Status::SUCCESS)); // TODO: Error handing.
}
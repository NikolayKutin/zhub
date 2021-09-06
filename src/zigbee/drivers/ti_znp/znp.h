// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "unp/unp.h"

#define DEFAULT_RESPONSE_TIMEOUT 3s // TODO: timeout.
#define SYNC_RESPONSE_TIMEOUT 3s
#define ASYNC_RESPONSE_TIMEOUT 3s
#define RESET_TIMEOUT 70s

#define DEFAULT_RADIUS 7

using namespace std::chrono;
using namespace zigbee;

namespace TiZnp {

    enum class Status {
        SUCCESS = 0x00,
        FAILURE = 0x01,
        INVALID_PARAMETR = 0x02,
        NV_ITEM_UNINIT = 0x09,
        NV_OPER_FAILED = 0x0a,
        NV_BAD_ITEM_LENGTH = 0x0c,
        MEMORY_ERROR = 0x10,
        BUFFER_FULL = 0x11,
        UNSUPPORTED_MODE = 0x12,
        MAC_MEMMORY_ERROR = 0x13,
        ZDO_INVALID_REQUEST_TYPE = 0x80,
        ZDO_INVALID_ENDPOINT = 0x82,
        ZDO_UNSUPPORTED = 0x84,
        ZDO_TIMEOUT = 0x85,
        ZDO_NO_MUTCH = 0x86,
        ZDO_TABLE_FULL = 0x87,
        ZDO_NO_BIND_ENTRY = 0x88,
        SEC_NO_KEY = 0xa1,
        SEC_MAX_FRM_COUNT = 0xa3,
        APS_FAIL = 0xb1,
        APS_TABLE_FULL = 0xb2,
        APS_ILLEGAL_REQEST = 0xb3,
        APS_INVALID_BINDING = 0xb4,
        APS_UNSUPPORTED_ATTRIBUTE = 0xb5,
        APS_NOT_SUPPORTED = 0xb6,
        APS_NO_ACK = 0xb7,
        APS_DUPLICATE_ENTRY  = 0xb8,
        APS_NO_BOUND_DEVICE = 0xb9,
        NWK_INVALID_PARAM = 0xc1,
        NWK_INVALID_REQUEST = 0xc2,
        NWK_NOT_PERMITTED = 0xc3,
        NWK_STARTUP_FAILURE = 0xc4,
        NWK_TABLE_FULL = 0xc7,
        NWK_UNKNOWN_DEVICE = 0xc8,
        NWK_UNSUPPORTED_ATTRIBUTE = 0xc9,
        NWK_NO_NETWORK = 0xca,
        NWK_LEAVE_UNCONFIRMED = 0xcb,
        NWK_NO_ACK = 0xcc,
        NWK_NO_ROUTE = 0xcd,
        MAC_NO_ACK = 0xe9
    };

    class Znp : public zigbee::Adapter {
    public:

        Znp() = delete;

        Znp(boost::asio::io_service &io_service);

        ~Znp();

        enum class ZdoState {
            NOT_STARTED = 0,            // Initialized - not started automatically
            NOT_CONNECTED = 1,          // Initialized - not connected to anything
            DISCOVERING = 2,            // Discovering PAN's to join
            JOINING = 3,                // Joining a PAN
            END_DEVICE_REJOINING = 4,   // Rejoining a PAN, only for end devices
            END_DEVICE_UNAUTH = 5,      // Joined but not yet authenticated by trust center
            END_DEVICE_ = 6,            // Started as device after authentication
            ROUTER = 6,                 // Device joined, authenticated and is a router
            COORDINATOR_STARTING = 8,   // Starting as ZigBee Coordinator
            COORDINATOR = 9,            // Started as ZigBee Coordinator
            LOST_PARENT = 10            // Device has lost information about its parent
        };

        enum class NvItems {
            STARTUP_OPTION = 0x0003, // 1 : default - 0, bit 0 - STARTOPT_CLEAR_CONFIG, bit 1 - STARTOPT_CLEAR_STATE (need reset).
            PAN_ID = 0x0083, // 2 : 0xFFFF to indicate �don�t care� (auto).
            EXTENDED_PAN_ID = 0x002D, // 8: (0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD) - from zigbee2mqtt
            CHANNEL_LIST = 0x0084, // 4 : channel bit mask. Little endian. Default is 0x00000800 for CH11;  Ex: value: [ 0x00, 0x00, 0x00, 0x04 ] for CH26, [ 0x00, 0x00, 0x20, 0x00 ] for CH15.
            LOGICAL_TYPE = 0x0087, // 1 : COORDINATOR: 0, ROUTER : 1, END_DEVICE : 2
            PRECFG_KEY = 0x0062, // 16 : (0x01, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0D, 0x0F, 0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0D) - from zigbee2mqtt 
            PRECFG_KEYS_ENABLE = 0x0063, // 1 : defalt - 1
            ZDO_DIRECT_CB = 0x008F, // 1 : defaul - 0, need for callbacks - 1
            ZNP_HAS_CONFIGURED = 0x0F00, // 1 : 0x55
        };

        enum class ResetType { HARD = 0, SOFT = 1 };

        bool reset(ResetType resetType, bool clearNetworkState, bool clearConfig);

        bool reset() { return reset(ResetType::SOFT, false, false); }

        bool startup(std::chrono::duration<int, std::milli> delay);

        bool startup() { return startup(100ms); }

        bool connect(std::string port, unsigned int baud_rate = 115200) { return unpi_.connect(port, baud_rate); }

        void disconnect() { unpi_.disconnect(); }

        bool ping();

        bool registerEndpoint(zigbee::SimpleDescriptor endpointDescriptor);

        bool permitJoin(const std::chrono::duration<int, std::ratio<1>> duration);

        std::vector<unsigned int> activeEndpoints(zigbee::NetworkAddress address);

        zigbee::SimpleDescriptor simpleDescriptor(zigbee::NetworkAddress address, unsigned int endpointNumber);

        std::vector<zigbee::NetworkAddress> associatedDevices();

        std::optional <int> setTransmitPower(int requestedPower); // Range: -22 ... 3 dBm.

        std::optional<std::pair<unsigned, unsigned>> version();

        bool sendMessage(zigbee::Message message);

        zigbee::NetworkAddress getNetworkAddress() { return network_address_; }
        zigbee::MacAddress getMacAddress() { return mac_address_; }

        zigbee::NetworkConfiguration readNetworkConfiguration();

        bool writeNetworkConfiguration(zigbee::NetworkConfiguration configuration);

        bool bind(zigbee::NetworkAddress device_network_address, zigbee::MacAddress source_mac_address, uint8_t source_endpoint,
            zigbee::MacAddress destination_mac_address, uint8_t destination_endpoint, zcl::Cluster cluster);

        bool unbind(zigbee::NetworkAddress device_network_address, zigbee::MacAddress source_mac_address, uint8_t source_endpoint,
            zigbee::MacAddress destination_mac_address, uint8_t destination_endpoint, zcl::Cluster cluster);

    private:

        enum Commands {
            SYS_RESET_REQ = 0x4100, SYS_RESET_IND = 0x4180,
            ZDO_STARTUP_FROM_APP = 0x2540, ZDO_STARTUP_FROM_APP_SRSP = 0x6540, ZDO_STATE_CHANGE_IND = 0x45c0,
            AF_REGISTER = 0x2400, AF_REGISTER_SRSP = 0x6400,
            AF_INCOMING_MSG = 0x4481,
            ZDO_MGMT_PERMIT_JOIN_REQ = 0x2536, ZDO_MGMT_PERMIT_JOIN_SRSP = 0x6536, ZDO_MGMT_PERMIT_JOIN_RSP = 0x45b6, ZDO_PERMIT_JOIN_IND = 0x45cb,
            ZDO_TC_DEV_IND = 0x45ca, ZDO_LEAVE_IND = 0x45c9, ZDO_END_DEVICE_ANNCE_IND = 0x45c1,
            ZDO_ACTIVE_EP_REQ = 0x2505, ZDO_ACTIVE_EP_SRSP = 0x6505, ZDO_ACTIVE_EP_RSP = 0x4585,
            ZDO_SIMPLE_DESC_REQ = 0x2504, ZDO_SIMPLE_DESC_SRSP = 0x6504, ZDO_SIMPLE_DESC_RSP = 0x4584,
            SYS_PING = 0x2101, SYS_PING_SRSP = 0x6101,
            SYS_OSAL_NV_READ = 0x2108, SYS_OSAL_NV_READ_SRSP = 0x6108,
            SYS_OSAL_NV_WRITE = 0x2109, SYS_OSAL_NV_WRITE_SRSP = 0x6109,
            SYS_OSAL_NV_ITEM_INIT = 0x2107, SYS_OSAL_NV_ITEM_INIT_SRSP = 0x6107,
            UTIL_GET_DEVICE_INFO = 0x2700, UTIL_GET_DEVICE_INFO_SRSP = 0x6700,
            SYS_SET_TX_POWER = 0x2114, SYS_SET_TX_POWER_SRSP = 0x6114,
            SYS_VERSION = 0x2102, SYS_VERSION_SRSP = 0x6102,
            AF_DATA_REQUEST = 0x2401, AF_DATA_REQUEST_SRSP = 0x6401, AF_DATA_CONFIRM = 0x4480,
            ZDO_BIND_REQ = 0x2521, ZDO_UNBIND_REQ = 0x2522, ZDO_BIND_RSP = 0x45a1, ZDO_UNBIND_RSP = 0x45a2
        };

        std::optional<TiUnp::Command> waitResponse(TiUnp::Command request, uint16_t id, std::chrono::duration<int, std::milli> timeout = DEFAULT_RESPONSE_TIMEOUT);

        std::optional<TiUnp::Command> syncRequest(TiUnp::Command request, std::chrono::duration<int, std::milli> timeout = SYNC_RESPONSE_TIMEOUT);

        std::optional<TiUnp::Command> asyncRequest(TiUnp::Command& request, uint16_t async_response_id, std::chrono::duration<int, std::milli> timeout = ASYNC_RESPONSE_TIMEOUT);

        bool verifySyncResponse(std::optional<TiUnp::Command> response);
        
        void handleCommand(TiUnp::Command command);

        std::optional<std::vector<uint8_t>> readNv(NvItems item);

        bool writeNv(NvItems item, std::vector<uint8_t> value);

        bool initNv(NvItems item, uint16_t length, std::vector<uint8_t> value);

        inline uint8_t genarateTransactionNumber() { static uint8_t transactionNumber = 0; return transactionNumber++; }

        zcl::Frame parseZclData(std::vector<uint8_t> data);

        void onDisconnect() { io_service_->post(boost::bind(boost::ref(OnDisconnect))); }

        boost::asio::io_service *io_service_ = nullptr;

        TiUnp::Unpi unpi_;

        utils::EventEmitter<TiUnp::Command> command_emitter_;

        ZdoState current_state_;

        std::pair<unsigned, unsigned> version_ { 0, 0 };

        zigbee::NetworkAddress network_address_ = 0;
        zigbee::MacAddress mac_address_ = 0;
	};

}
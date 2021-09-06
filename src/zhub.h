// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/tokenizer.hpp>
#include <mqtt_client_cpp.hpp>

#include "zigbee/zigbee.h"
#include "functional_unit.h"

#include "json.h"
#include "utils/log.h"

class Zhub {
public:

	Zhub() = delete;
	Zhub(boost::asio::io_service& io_service) : io_service_(&io_service), adapter_(std::make_unique<TiZnp::Znp>(io_service)),
		coordinator_(adapter_.get(), &io_service) {}
	~Zhub() {
		adapter_->disconnect();
		mqtt_client_->disconnect();
	}

    void init_from_yaml(std::string config_file_name);

private:

    using MqttClient = typename mqtt::callable_overlay<mqtt::sync_client<mqtt::tcp_endpoint<boost::asio::ip::tcp::socket,
        boost::asio::io_context::strand>>>;
    using packet_id_t = MqttClient::packet_id_t;

    void setZigbeeAdapterCallbacks();
    void setZigbeeCoordinatorCallbacks();
    void setMqttClientCallbacks();
    void initZigbee();
    void initMqtt();
    void createFunctionalUnit(FunctionalUnit::Descriptor& descriptor);
    void updateConfiguration();

    bool onMqttPublish(MQTT_NS::optional<packet_id_t> packet_id, MQTT_NS::publish_options pubopts, MQTT_NS::buffer topic_name,
        MQTT_NS::buffer contents);
    bool onMqttConnack(bool sp, MQTT_NS::connect_return_code connack_return_code);
    void mqttReconnect();
    void mqttConnect();

    struct MqttTopicPath {
        std::string type;
        std::string name;
        std::string channel;
    };

    void loadConfigurationFromYaml(const std::string& config_file_name);
    void saveConfigurationToYaml(const std::string& config_file_name);

    MqttTopicPath parseMqttTopic(const std::string& topic, const std::string& root_topic);
    void executeServiceCommand(std::string& command);

	boost::asio::io_service* io_service_;

	std::unique_ptr<zigbee::Adapter> adapter_;
	zigbee::Coordinator coordinator_;

	std::shared_ptr<MqttClient> mqtt_client_;

    std::map<std::pair<std::string, std::string>, std::shared_ptr<FunctionalUnit>> units_; // key: <type, id>

    std::string config_file_name_;

    struct {
        struct {
            std::string serial;
            unsigned int baudrate;
        } zigbee;

        struct {
            std::string host;
            unsigned port;
            std::string user_name;
            std::string password;
            std::string root_topic;
        } mqtt;

        std::list<FunctionalUnit::Descriptor> unit_descriptors;
    } configuration_;
};
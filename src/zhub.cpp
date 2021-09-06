// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#include "zhub.h"

void Zhub::initMqtt() {
    LOG_INFO << "Connecting to mqtt broker " << configuration_.mqtt.host << ":" << configuration_.mqtt.port;
    mqtt_client_ = MQTT_NS::make_sync_client(*io_service_, configuration_.mqtt.host, std::to_string(configuration_.mqtt.port));

    setMqttClientCallbacks();

    mqtt_client_->set_user_name(configuration_.mqtt.user_name);
    mqtt_client_->set_password(configuration_.mqtt.password);
    mqtt_client_->set_clean_session(true);

    mqttConnect();
}

void Zhub::initZigbee() {
    setZigbeeAdapterCallbacks();
    setZigbeeCoordinatorCallbacks();

    LOG_INFO << "Connecting to serial(" << configuration_.zigbee.serial << ", " << configuration_.zigbee.baudrate << "bps)";
    if (!adapter_->connect(configuration_.zigbee.serial, configuration_.zigbee.baudrate))
        throw std::runtime_error("Failure connect to serial");

    LOG_INFO << "Starting zigbee coordinator";
    coordinator_.start();
}

void Zhub::updateConfiguration() {
    try {
        configuration_.unit_descriptors.clear();
        for (auto& it : units_)
            configuration_.unit_descriptors.push_back(std::move(it.second->getDescriptor()));

        saveConfigurationToYaml(config_file_name_);
        LOG_INFO << "Unit configuration saved";
    }
    catch (std::exception& error) {
        LOG_ERROR << "Configuration saving error: " << error.what();
    }
}

void Zhub::createFunctionalUnit(FunctionalUnit::Descriptor& descriptor) {
    auto unit = FunctionalUnit::createUnit(descriptor, &coordinator_);

    if (unit) {
        units_.insert(std::make_pair(std::make_pair(descriptor.type, descriptor.id), unit));

        unit->OnMessage.connect([this](FunctionalUnit* unit, std::string channel, std::string command) {
            mqtt_client_->publish(configuration_.mqtt.root_topic + "/" + unit->type() + "/" + unit->id() + "/" + channel,
                command, MQTT_NS::qos::at_most_once);
            });

        unit->OnError.connect([](FunctionalUnit* unit, std::string message) {
            LOG_ERROR << "Unit /" << unit->type() << "/" << unit->id() << ": " << message;
            });

        unit->OnSettingsChange.connect([this](FunctionalUnit*) {
            try {
                updateConfiguration();
            }
            catch (std::runtime_error error) {
                LOG_ERROR << error.what();
            }
            });

        LOG_INFO << toJson(*unit);
    }
}

bool Zhub::onMqttPublish(MQTT_NS::optional<packet_id_t> packet_id, MQTT_NS::publish_options pubopts, MQTT_NS::buffer topic_name, MQTT_NS::buffer contents) {
    std::string topic = topic_name.to_string();
    std::string message = contents.to_string();
    static std::string zhub_topic = configuration_.mqtt.root_topic + "/zhub";

    try {
        if (topic == zhub_topic)
            executeServiceCommand(message);
        else {
            MqttTopicPath patch = parseMqttTopic(topic, configuration_.mqtt.root_topic);
            units_.at(std::make_pair(patch.type, patch.name))->executeCommand(patch.channel, message);
        }
    }
    catch (std::invalid_argument) {} // TODO ? parsing error handling
    catch (std::out_of_range) {
        LOG_DEBUG << "Incoming mqtt message from unknown unit: " << topic_name;
    }

    return true;
}

Zhub::MqttTopicPath Zhub::parseMqttTopic(const std::string& topic, const std::string& root_topic) {
    // Topic format: root_topic/unit_type/unit_name/channel
    // throw std::invalid_argument if format is incorrect or path have incorrect element

    boost::tokenizer<boost::char_separator<char>> tokenizer(topic, boost::char_separator<char>("/"));
    std::vector<std::string> tokens(tokenizer.begin(), tokenizer.end());

    MqttTopicPath path;
    try {
        if (tokens.size() > 4) // root_topic/unit_type/unit_name/subtopic
            throw std::invalid_argument("too long path");

        if (tokens.at(0) != root_topic)
            throw std::invalid_argument("topic does not start with root_topic");

        path.type = tokens.at(1);
        if (path.type.empty())
            throw std::invalid_argument("incorrect type");

        path.name = tokens.at(2);
        if (path.name.empty())
            throw std::invalid_argument("incorrect name");

        path.channel = tokens.at(3);
        if (path.channel.empty())
            throw std::invalid_argument("incorrect subtopic");
    }
    catch (std::out_of_range) {
        throw std::invalid_argument("too short path");
    }

    return path;
}

void Zhub::setMqttClientCallbacks() {
    mqtt_client_->set_close_handler([]() {
        LOG_WARNING << "Mqtt connection closed";
        });

    mqtt_client_->set_error_handler([](MQTT_NS::error_code ec) {
        LOG_ERROR << "Mqtt client error: " << ec.message();
        });

    mqtt_client_->set_connack_handler(boost::bind(&Zhub::onMqttConnack, this, boost::placeholders::_1, boost::placeholders::_2));

    mqtt_client_->set_suback_handler([this](packet_id_t packet_id, std::vector<MQTT_NS::suback_return_code> results) {
        if (results.size() && (results[0] != MQTT_NS::suback_return_code::failure)) {
            LOG_INFO << "Subscribed to topic: " << configuration_.mqtt.root_topic << "/#";
        }
        else LOG_ERROR << "Failure subscribe to topic: " << configuration_.mqtt.root_topic << "/#";

        return true;
        });

    mqtt_client_->set_publish_handler(boost::bind(&Zhub::onMqttPublish, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4));
}

void Zhub::setZigbeeCoordinatorCallbacks() {
    coordinator_.OnLog.connect([](std::string message) {
        LOG_INFO << message;
        });
    coordinator_.OnError.connect([](std::runtime_error error) {
        LOG_ERROR << error.what();
        });
    coordinator_.OnJoin.connect([this](std::shared_ptr<zigbee::EndDevice> device) {
        LOG_INFO << device->model() << " (network address - 0x" << std::hex << device->getNetworkAddress()
            << ", ieee address - 0x" << device->getMacAddress() << ") connected to coordinator";

        mqtt_client_->publish(configuration_.mqtt.root_topic + "/zhub/devices", toJson(*device), MQTT_NS::qos::at_most_once);
        });

    coordinator_.OnLeave.connect([](zigbee::NetworkAddress address) {
        LOG_INFO << "Device [0x" << std::hex << address << "] disconnected from coordinator";
        });

    coordinator_.OnPermitJoin.connect([](std::chrono::duration<int, std::ratio<1>> duration) {
        LOG_INFO << "Join permitted for " << duration.count() << " seconds";
        });

    coordinator_.OnPermitJoinCompletion.connect([]() {
        LOG_INFO << "Permit join completed";
        });
}

void Zhub::setZigbeeAdapterCallbacks() {
    adapter_->OnDisconnect.connect([]() {
        LOG_ERROR << "Zigbee adapter disconnected";
        });
    adapter_->OnJoin.connect([](zigbee::NetworkAddress network_address, zigbee::MacAddress mac_address) {
        LOG_INFO << "Zigbee end device joined (network address - 0x" << std::hex << network_address << ", ieee address - 0x" << mac_address << ")";
        });
    adapter_->OnLeave.connect([](zigbee::NetworkAddress network_address, zigbee::MacAddress mac_address) {
        LOG_INFO << "Zigbee end device leaved (network address - 0x" << std::hex << network_address << ", ieee address - 0x" << mac_address << ")";
        });
}

void Zhub::init_from_yaml(std::string config_file_name) {
    LOG_INFO << "Zhub version 0.01";

    std::stringstream sstream;
    sstream << "Supported zigbee device models: ";
    for (auto& model : zigbee::EndDevice::getSupportedModels())
        sstream << model << " ";
    LOG_INFO << sstream.str();

    config_file_name_ = std::move(config_file_name);
    LOG_INFO << "Loading configuration from " << config_file_name_;
    loadConfigurationFromYaml(config_file_name_);

    initZigbee();
    initMqtt();

    LOG_INFO << "Add functional units: ";
    for (auto& descriptor : configuration_.unit_descriptors)
        createFunctionalUnit(descriptor);
}

bool Zhub::onMqttConnack(bool sp, MQTT_NS::connect_return_code connack_return_code) {
    if (connack_return_code == MQTT_NS::connect_return_code::accepted) {
        LOG_INFO << "Mqtt connection estabilished";

        mqtt_client_->subscribe(configuration_.mqtt.root_topic + "/#", MQTT_NS::qos::at_most_once);

        for (auto unit_pair : units_)
            mqtt_client_->publish(configuration_.mqtt.root_topic + "/zhub/units", toJson(*unit_pair.second), MQTT_NS::qos::at_most_once);

        for (auto device : coordinator_.GenericDevices())
            mqtt_client_->publish(configuration_.mqtt.root_topic + "/zhub/devices", toJson(*device), MQTT_NS::qos::at_most_once);
    }
    else {
        LOG_ERROR << "MQTT connection error: " << MQTT_NS::connect_return_code_to_str(connack_return_code);
        io_service_->post(boost::bind(&Zhub::mqttReconnect, this));
    }

    return true;
}

void Zhub::mqttReconnect() {
    static boost::asio::deadline_timer timer(*io_service_);

    timer.expires_from_now(boost::posix_time::seconds(15));
    timer.async_wait([this](const boost::system::error_code& ec) {
        if (!ec) {
            LOG_INFO << "Reconnecting to mqtt broker " << configuration_.mqtt.host << ":" << configuration_.mqtt.port;
            mqttConnect();
        }
        });
}

void Zhub::mqttConnect() {
    try {
        mqtt_client_->connect();
    }
    catch (boost::system::system_error& error) {
        LOG_ERROR << "MQTT connection error: " << error.code().message();
        io_service_->post(boost::bind(&Zhub::mqttReconnect, this));
    }
}

void Zhub::executeServiceCommand(std::string& command) {
    if (command == "devices") {
        for (auto device : coordinator_.GenericDevices())
            mqtt_client_->publish(configuration_.mqtt.root_topic + "/zhub/devices", toJson(*device), MQTT_NS::qos::at_most_once);
    }
    else if (command == "units") {
        for (auto unit_pair : units_)
            mqtt_client_->publish(configuration_.mqtt.root_topic + "/zhub/units", toJson(*unit_pair.second), MQTT_NS::qos::at_most_once);
    }
    else if (command == "public") {
        for (auto unit_pair : units_)
            unit_pair.second->publicAll();
    }
}
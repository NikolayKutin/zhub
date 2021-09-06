// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#include "zigbee.h"

using namespace zigbee;

utils::GenericObjectFactory<std::string, EndDevice, zigbee::NetworkAddress, zigbee::MacAddress, Coordinator*>
    utils::FactoryClass<zigbee::EndDevice, zigbee::NetworkAddress, zigbee::MacAddress, zigbee::Coordinator*>::factory_;

std::map<std::string, std::string> EndDevice::model_to_identifier_table;

std::shared_ptr<zigbee::EndDevice> EndDevice::createDevice(std::string identifier, zigbee::Coordinator* coordinator, zigbee::NetworkAddress network_address, zigbee::MacAddress mac_address) {
    return EndDevice::createInstance(identifier, network_address, mac_address, coordinator); }

std::string EndDevice::getIdentifier(std::string model) {
    if (EndDevice::model_to_identifier_table.count(model))
        return model_to_identifier_table[model];
    else
        return std::string();
}

EndDevice::EndDevice(zigbee::NetworkAddress network_address, zigbee::MacAddress mac_address, zigbee::Coordinator *coordinator) : network_address_(network_address), mac_address_(mac_address), coordinator_(coordinator) {
    on_attribute_report_connection_ = coordinator_->OnAttributeReport.connect(
        boost::bind(&EndDevice::onCoordinatorAttributeReport, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));
    on_leave_connection_ = coordinator_->OnLeave.connect(
        boost::bind(&EndDevice::onCoordinatorLeave, this, boost::placeholders::_1));
}

EndDevice::~EndDevice() { on_attribute_report_connection_.disconnect(); on_leave_connection_.disconnect(); }

zcl::Attribute EndDevice::readAttribute(uint8_t endpiont_number, zcl::Cluster cluster, uint16_t id) {
    return coordinator_->readAttribute(zigbee::Endpoint { network_address_, endpiont_number}, cluster, id); }

zcl::Status EndDevice::writeAttribute(uint8_t endpiont_number, zcl::Cluster cluster, zcl::Attribute attribute) {
    return coordinator_->writeAttribute(zigbee::Endpoint { network_address_, endpiont_number}, cluster, attribute); }

zcl::Status EndDevice::sendCommand(uint8_t endpiont_number, zcl::Cluster cluster, uint8_t id, std::vector<uint8_t> payload) {
    return coordinator_->sendCommand(zigbee::Endpoint { network_address_, endpiont_number}, cluster, id, payload); }

void EndDevice::onCoordinatorAttributeReport(zigbee::Endpoint endpoint, zcl::Cluster cluster, zcl::Attribute attribute) {
    if (endpoint.address == network_address_)
        onAttributeReport(endpoint.number, cluster, attribute);
}

void EndDevice::onCoordinatorLeave(zigbee::NetworkAddress network_address) {
    if (network_address == network_address_) {
        leaved_ = true;
        OnLeave();
    }
}

bool EndDevice::bind(uint8_t device_endpoint, uint8_t coordinator_endpoint, zcl::Cluster cluster) {
    return coordinator_->bind(this, device_endpoint, coordinator_, coordinator_endpoint, cluster); }

bool EndDevice::unbind(uint8_t device_endpoint, uint8_t coordinator_endpoint, zcl::Cluster cluster) {
    return coordinator_->unbind(this, device_endpoint, coordinator_, coordinator_endpoint, cluster); }

bool EndDevice::configureReporting(uint8_t endpoint, zcl::Cluster cluster, uint16_t attribute_id, zcl::Attribute::DataType attribute_data_type,
    std::chrono::duration<uint16_t, std::ratio<1>> min_interval, std::chrono::duration<uint16_t, std::ratio<1>> max_interval, std::vector<uint8_t> reportable_change) {
    return coordinator_->configureReporting({ network_address_, endpoint }, cluster, attribute_id, attribute_data_type, min_interval, max_interval, reportable_change); }

std::vector<std::string> EndDevice::getSupportedModels() {
    std::vector<std::string> models;

    for (auto item : model_to_identifier_table)
        models.push_back(item.first);

    return models;
}
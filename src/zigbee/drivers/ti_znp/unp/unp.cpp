// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#include "../../../zigbee.h"

using namespace TiUnp;

Unpi::Unpi(boost::asio::io_service &io_service) : serial_(io_service), io_service_(&io_service) {
    receive_buffer_.reserve(RX_BUFFER_SIZE);
    transmit_buffer_.reserve(TX_BUFFER_SIZE);
}

Unpi::~Unpi() { disconnect(); }

bool Unpi::connect(std::string port, unsigned int baud_rate) {
    if (serial_.is_open()) return false;

    try {
        serial_.open(port);

        serial_.set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
        serial_.set_option(boost::asio::serial_port_base::character_size(8));
        serial_.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
        serial_.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));

        execute_flag_ = true;

        receiver_thread_ = std::thread(&Unpi::loop, this);
        receiver_thread_.detach();
    }
    catch (boost::system::system_error error) {
        disconnect();
        return false;
    }

    return true;
}

void Unpi::disconnect() {
    execute_flag_ = false;

    if (serial_.is_open()) {
        try {
            serial_.close();
            io_service_->post(boost::bind(boost::ref(OnDisconnect)));
        }
        catch (boost::system::system_error error) {} // TODO: Error handling?
    }
}

std::vector<Command> Unpi::parseReceivedData(std::vector<uint8_t>& data) {
    std::vector<Command> parsed_commands;

    try {
        for (size_t i = 0; i < data.size();) {
            if (data.at(i++) == SOF) {
                size_t payload_length = data.at(i++);

                uint8_t cmd0 = data.at(i++);
                uint8_t cmd1 = data.at(i++);

                if ((payload_length <= PAYLOAD_MAX_LENGTH) && ((i + payload_length) <= data.size())) {
                    Command command(_UINT16(cmd1, cmd0), payload_length);

                    std::copy_n(&data[i],  payload_length, command.data());
                    i += payload_length;

                    if (data.at(i++) == command.fcs())
                        parsed_commands.push_back(std::move(command));
                }
            }
        }
    }
    catch (std::out_of_range) { std::cerr << "Bad received data (Unpi)" << std::endl; } // TODO: Error handling (receiver thread).

    receive_buffer_.clear(); // TODO: Switch data to receive_buffer_.

    return parsed_commands;
}



bool Unpi::sendCommand(Command command) {
    if (!serial_.is_open()) return false;

    transmit_buffer_.clear();

    std::lock_guard<std::mutex> transmit_lock(transmit_mutex_);

    transmit_buffer_.push_back(SOF);                                                            // SOF
    transmit_buffer_.push_back(LOWBYTE(command.payload_size()));                                // Length
    transmit_buffer_.push_back(HIGHBYTE(command.id()));                                         // CMD0
    transmit_buffer_.push_back(LOWBYTE(command.id()));                                          // CMD1

    std::copy_n(command.data(), command.payload_size(), std::back_inserter(transmit_buffer_));  // Payload

    transmit_buffer_.push_back(command.fcs());                                                  // FCS

    return (serial_.write_some(boost::asio::buffer(transmit_buffer_)) == transmit_buffer_.size());
}

void Unpi::loop() {
    try {
        while (execute_flag_) {
            receive_buffer_.resize(receive_buffer_.capacity());
            receive_buffer_.resize(serial_.read_some(boost::asio::buffer(receive_buffer_)));

            for (auto command : parseReceivedData(receive_buffer_))
                OnCommand(command);
        }
    }
    catch (boost::system::system_error error) { // TODO: Error handling?
        disconnect();
    }
}
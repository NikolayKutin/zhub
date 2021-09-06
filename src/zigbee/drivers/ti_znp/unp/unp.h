// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "command.h"

#define SOF 0xFE

#define RX_BUFFER_SIZE 1024
#define TX_BUFFER_SIZE 256

namespace TiUnp {

    class Unpi {
    public:

        Unpi() = delete;

        explicit Unpi(boost::asio::io_service &io_service);

        ~Unpi();

        bool connect(std::string port, unsigned int baud_rate);

        void disconnect();

        boost::signals2::signal<void(Command)> OnCommand;
        boost::signals2::signal<void(void)> OnDisconnect;

        bool sendCommand(Command command);

    private:

        std::vector<Command> parseReceivedData(std::vector<uint8_t>& data);

        void loop();

        std::thread receiver_thread_;
        std::atomic_bool execute_flag_;

        boost::asio::io_service *io_service_;
        boost::asio::serial_port serial_;

        std::vector<uint8_t> transmit_buffer_;
        std::vector<uint8_t> receive_buffer_;

        std::mutex transmit_mutex_;
    };
}

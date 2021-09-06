// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define PAYLOAD_MAX_LENGTH 250

namespace TiUnp {

    enum class Subsystem { RPC_ERROR = 0, SYS = 1, MAC = 2, NWK = 3, AF = 4, ZDO = 5, SAPI = 6, UTIL = 7, DEBURG = 8, APP = 9, APP_CNF = 15, GREENPOWER = 21 };

    class Command {
    public:
        enum class Type { POLL = 0, SREQ = 1, AREQ = 2, SRSP = 3 };

        Command() : Command(0, 0) {}

        Command(uint16_t id) : id_(id) {
            payload_ = std::make_shared<std::vector<uint8_t>>();
            payload_->reserve(PAYLOAD_MAX_LENGTH);
        }

        Command(uint16_t id, size_t payload_length) : id_(id) {
            payload_ = std::make_shared<std::vector<uint8_t>>();
            payload_->resize(payload_length);
        }

        ~Command() {}

        inline uint8_t& payload(const size_t index) {
            assert(index < PAYLOAD_MAX_LENGTH);

            if (index >= payload_->size())
               payload_->resize(index + 1);

            return payload_->at(index);
        }

        uint8_t fcs() {
            uint8_t _fcs = static_cast<uint8_t>(payload_->size()) ^ HIGHBYTE(id_) ^ LOWBYTE(id_);

            for (auto byte : *payload_)
                _fcs ^= byte;

            return _fcs;
        }

        //inline operator bool() const { return (_id); }

        inline bool operator== (Command const& command) { return (command.id_ == id_); }

        inline bool operator!= (Command const& command) { return !(*(this) == command); }

        inline Type type() { return static_cast<Type>(HIGHBYTE(id_) >> 5); }

        inline Subsystem subsystem() { return static_cast<Subsystem>(HIGHBYTE(id_) & 0b00011111); }

        inline uint16_t id() { return id_; }

        inline size_t payload_size() { return payload_->size(); }

        inline uint8_t* data() { return payload_->data(); }

    private:

        std::shared_ptr<std::vector<uint8_t>> payload_;
        uint16_t id_;
    };
}
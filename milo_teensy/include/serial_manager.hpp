#ifndef SERIAL_MANAGER_H
#define SERIAL_MANAGER_H

#include <Arduino.h>
#include <cstdint>
#include <cstddef>

#include <pb_decode.h>
#include "RoverCommand.pb.h"

class SerialManager
{
public:
    // 1. Call updateSerial(), then consumeGetLatestCommand() immediately after
    void updateSerial(); // Called in every loop by main - read bytes and decodes message

    bool consumeLatestCommand(RoverCommand& out_cmd); // Returns newest RoverCommand if available

private:
    // TODO (james): Does this belong in private or outside of the class? Also make sure there are no dangling variables/all intiailised in constructor
    enum class RxState
    {
        WaitSync1,
        WaitSync2,
        ReadLengthHigh,
        ReadLengthLow,
        ReadPayload,
        ReadCrcHigh,
        ReadCrcLow
    };


    void processByte(uint8_t byte);

    void resetFrameParser(); // Resets state and state variables

    bool decodePayload(); // Decodes the serial payload via nanopb

    static uint16_t updateCRC(uint16_t current_crc, uint8_t byte);

    // TODO (james): Future methods for serialising payload and writing back to sbc

    // TODO (james): Move these to config.hpp
    // CRC constants
    static constexpr uint8_t sync1_ = 0xAA; // Sync bits 0XAA55
    static constexpr uint8_t sync2_ = 0x55;
    static constexpr uint16_t crc16_ccitt_init_ = 0xFFFF;
    static constexpr uint16_t crc16_ccitt_msb_ = 0x8000;
    static constexpr uint16_t crc16_ccitt_polynomial_ = 0x1021;

    static constexpr std::size_t max_payload_size_ = 256;
    uint16_t expected_payload_length_ = 0;
    uint16_t payload_idx_ = 0;
    uint8_t payload_buffer_[max_payload_size_]{};

    uint16_t running_crc_ = crc16_ccitt_init_;
    uint16_t received_crc_ = 0;

    RxState rx_state_ = RxState::WaitSync1; // Default state is waiting for sync1
    RoverCommand latest_command_ = RoverCommand_init_zero;

    bool has_new_command_ = false;

};

#endif // SERIAL_MANAGER_H
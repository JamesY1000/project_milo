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
    void updateSerial(); // Called in every loop by main - read bytes and decodes message

    bool newCmdReceived(); // Called in update to check if a new message has been received

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

    // Processes one incoming byte from the serial stream.
    // Bytes are fed through a state machine which reconstructs a complete
    // [sync][length][payload][crc16] frame. Once a valid frame is received,
    // the protobuf payload is deserialized into a RoverCommand message.
    void processByte(uint8_t byte);

    void resetFrameParser();

    bool decodePayload(); // Decodes the serial payload via nanopb

    static uint16_t updateCRC(uint16_t current_crc, uint8_t byte);

    uint16_t calculateCRC(const uint8_t *data, size_t length) const;

    // CRC constants
    static constexpr uint8_t sync1_ = 0xAA; // Sync bits 0XAA55
    static constexpr uint8_t sync2_ = 0x55;
    static constexpr uint8_t crc16_ccitt_init_ = 0xFFFF;
    static constexpr uint8_t crc16_ccitt_msb_ = 0x8000;
    static constexpr uint8_t crc16_ccitt_polynomial_ = 0x1021;

    // static constexpr std::size_t max_payload_size_ = 256;

    RxState rx_state_; // Should start in WaitSync1

    uint16_t running_crc_; // Should be crc initial value (crc16_ccitt_init)
    uint16_t received_crc_;

    RoverCommand latest_command_;
};

#endif // SERIAL_MANAGER_H
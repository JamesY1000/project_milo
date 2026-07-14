#ifndef COMMAND_MANAGER__COMMAND_MANAGER_HPP_
#define COMMAND_MANAGER__COMMAND_MANAGER_HPP_

#include <Arduino.h>
// #include "proto/rover_command.pb.h"


class CommandManager
{
public:
    CommandManager();



private:
    ParseResult parseByte(uint8_t rx_byte); // One byte for one state transition - sync hunt, length, payload, crc

    void resetParser(); // Looks for 0xAA again. Called on crc failure, bad length, and frame timeout (set certain time so that even if it's mid frame, if it hasn't received a byte in say 50ms it will reset to look for a new frame).

    bool decodePayload(const uint8_t *buffer, uint16_t len) const; // pb_is_stream_from_buffer + pb_decode

    uint16_t crc16_ccitt(const uint8_t *data, size_t len) const; // Must mirror the crc in hardware_interface

    void sanitise(RoverCommand &cmd) const; // Clamps the wheel_* to [-1, 1] znd steer_* to mechanical limits (servo limits?)




};




#endif // COMMAND_MANAGER__COMMAND_MANAGER_HPP_
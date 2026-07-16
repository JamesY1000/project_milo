#ifndef SERIAL_MANAGER_H
#define SERIAL_MANAGER_H

#include <cstdint>
#include <cstddef>

class SerialManager
{
public:
    void updateSerial(); // Called in every loop by main - read bytes and decodes message

    bool newCmdReceived(); // Called in update to check if a new message has been received

private:
    enum class RxState
    {
        WaitSync1,
        WaitSync2,
        ReadLengthLow,
        ReadLengthHigh,
        ReadPayload,
        ReadCrcLow,
        ReadCrcHigh
    };

    void processByte(uint8_t byte);

    void resetFrameParser();

    bool decodePayload(); // Decodes the serial payload via nanopb

    uint16_t calculateCRC(const uint8_t *data, size_t length) const;

    static constexpr uint8_t sync1 = 0xAA; // Sync bits 0XAA55
    static constexpr uint8_t sync2 = 0x55;
    static constexpr uint8_t crc16_ccitt_init = 0xFFFF;
    static constexpr uint8_t crc16_ccitt_msb = 0x8000;
    static constexpr uint8_t crc16_ccitt_polynomial = 0x1021;
};

#endif // SERIAL_MANAGER_H
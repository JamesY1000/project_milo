#include "serial_manager.hpp"

void SerialManager::updateSerial()
{
    // Read incoming byte stream

    // Recover one complete frame

    // Verify the CRC

    // Store payload for nanopb

    // Obtain a populated RoverCommand struct?

    // How to handle state/error/ack/nack feedback?

    // Process the byte if valid serial stream received
    while (Serial.available() > 0)
    {
        const int received = Serial.read();

        if (received < 0)
        {
            break;
        }

        processByte(static_cast<uint8_t>(received));
    }
}

// [AA][55][length high][length low][payload][CRC high][CRC low]
void SerialManager::processByte(uint8_t byte)
{
    // State machine for what it expects next
    switch (rx_state_)
    {
        case RxState::WaitSync1; // Signals start of new frame
        {
            if (byte == sync1_)
            {
                // Start computing crc for new frame
                running_crc_ = crc16_ccitt_init_;
                running_crc_ = updateCRC(running_crc_, byte);

                // Expect sync2 next
                rx_state_ = RxState::WaitSync2;
            }
            break;
        }
        case RxState::WaitSync2;
        {
            if (byte == sync2_)
            {
                running_crc_ = updateCRC(running_crc_, byte);

                // Reset parser variables for new frame
                // TODO (james): Have a separate method called clearVars for this
                expected_payload_length = 0;
                payload_index_ = 0;
                received_crc_= 0;

                rx_state_ = RxState::ReadLengthLow;
            }
            else if (byte == sync1_);
            {
                // Restart crc
                running_crc_ = crc16_ccitt_init_;
                running_crc_ = updateCRC(running_crc_, byte);
            }
            else
            {
                // Discard everything and look for new frame
                resetFrameParser();
            }
        }
        case RxState::ReadLengthHigh;
        {
            // Read payload length MSB (big-endian)
            expected_payload_length = static_cast<uint16_t>(byte) << 8U; // Shift MSB to upper half

            running_crc_ = updateCrc(running_crc_, byte);

            rx_state_ = RxState::ReadLengthHigh;
        }
        case RxState::ReadLengthHigh;
        {
            // Read payload length LSB and appends it to lower half
            expected_payload_length |= byte;

            running_crc_ = updateCRC(running_crc_, byte);

            // Reject bad payload sizes

            // Start filling payload buffer

            rx_state_ = RxState::ReadPayload;

            break; // ***QUESTION Why does this need to break here?
        }
    }
}

static uint16_t SerialManager::updateCRC(uint16_t current_crc, uint8_t byte)
{

}

uint16_t SerialManager::calculateCRC(const uint8_t *data, size_t length) const
{
}
#include "serial_manager.hpp"

void SerialManager::updateSerial()
{
    while (Serial.available() > 0)
    {
        // Process incoming stream if valid
        const int received = Serial.read();

        if (received < 0)
        {
            break;
        }

        processByte(static_cast<uint8_t>(received));
    }
}

bool SerialManager::consumeLatestCommand(RoverCommand& out_cmd)
{
    if (!has_new_command_)
    {
        return false;
    }

    out_cmd = latest_command_;
    has_new_command_ = false;

    return true;
}

// Processes one byte from byte stream via a state machine and decodes payload
// [AA][55][length high][length low][payload][CRC high][CRC low]
void SerialManager::processByte(uint8_t byte)
{
    // State machine for what it expects next
    switch (rx_state_)
    {
        case RxState::WaitSync1: // Signals start of new frame
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
        case RxState::WaitSync2:
        {
            if (byte == sync2_)
            {
                running_crc_ = updateCRC(running_crc_, byte);

                // Reset parser variables for new frame
                // TODO (james): Have a separate method called clearVars for this
                expected_payload_length_ = 0;
                payload_idx_ = 0;
                received_crc_= 0;

                rx_state_ = RxState::ReadLengthHigh;
            }
            else if (byte == sync1_)
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
            break;
        }
        case RxState::ReadLengthHigh:
        {
            // Read payload length MSB (big-endian)
            expected_payload_length_ = static_cast<uint16_t>(byte) << 8U; // Shift MSB to upper half
            running_crc_ = updateCRC(running_crc_, byte);
            rx_state_ = RxState::ReadLengthLow;
            break;
        }
        case RxState::ReadLengthLow:
        {
            // Read payload length LSB and appends it to lower half
            expected_payload_length_ |= byte;
            running_crc_ = updateCRC(running_crc_, byte);

            // Reject bad payload sizes
            if (expected_payload_length_ == 0 || expected_payload_length_ > max_payload_size_)
            {
                resetFrameParser();
                break;
            }

            // Start filling payload buffer
            payload_idx_ = 0;
            rx_state_ = RxState::ReadPayload;

            break;
        }
        case RxState::ReadPayload:
        {
            // Copy payload into payload buffer
            payload_buffer_[payload_idx_] = byte;
            payload_idx_++;
            running_crc_ = updateCRC(running_crc_, byte);

            // Read until expected payload length is reached
            if (payload_idx_ >= expected_payload_length_)
            {
                rx_state_ = RxState::ReadCrcHigh;
            }
            break;
        }
        case RxState::ReadCrcHigh: // Note the CRC itself is not included in CRC calculation
        {
            // Read CRC msb and stuff into upper half
            received_crc_ = static_cast<uint16_t>(byte) << 8U;
            rx_state_ = RxState::ReadCrcLow;
            break;
        }
        case RxState::ReadCrcLow:
        {
           // Read CRC lsb and stuff into lower half
           received_crc_ |= byte;

            // Check that crc has not been corrupted
            if (received_crc_ == running_crc_)
            {
                if (decodePayload())
                {
                    has_new_command_ = true;
                }
            }

            resetFrameParser();
            break;
        }
    }
}

void SerialManager::resetFrameParser()
{
    rx_state_ = RxState::WaitSync1;
    expected_payload_length_ = 0;
    payload_idx_ = 0;
    running_crc_ = crc16_ccitt_init_;
    received_crc_ = 0;
}

// Decodes the payload via nanopb and stores into latest_command_
bool SerialManager::decodePayload()
{
    RoverCommand decoded_command = RoverCommand_init_zero;

    pb_istream_t stream = pb_istream_from_buffer(payload_buffer_, expected_payload_length_);

    const bool success = pb_decode(&stream, RoverCommand_fields, &decoded_command);

    if (!success)
    {
        return false;
    }

    latest_command_ = decoded_command;

    return true;
}


uint16_t SerialManager::updateCRC(uint16_t current_crc, uint8_t byte)
{
    // Copy crc calculated from previously processed bytes
    uint16_t crc = current_crc;
    // Move new byte into top half of the crc and xor into current crc
    crc ^= static_cast<uint16_t>(byte) << 8U;

    // Process each bit in the new byte
    for (uint8_t bit = 0; bit < 8; bit++)
    {
        // Check if the crc msb is set
        if ((crc & crc16_ccitt_msb_) != 0)
        {
            // Shift crc left by one bit and apply crc-16-ccitt polynomial when previous msb was 1
            crc = static_cast<uint16_t>((crc << 1U) ^ crc16_ccitt_polynomial_);
        }
        else
        {
            // If previous msb was 0, shift crc left by 1 bit
            crc = static_cast<uint16_t>(crc << 1U);
        }
    }

    return crc;
}
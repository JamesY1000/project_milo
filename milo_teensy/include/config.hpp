#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>


// Maybe group these for the serial manager?
static constexpr uint8_t sync1_ = 0xAA; // Sync bits 0XAA55
static constexpr uint8_t sync2_ = 0x55;
static constexpr uint16_t crc16_ccitt_init_ = 0xFFFF;
static constexpr uint16_t crc16_ccitt_msb_ = 0x8000;
static constexpr uint16_t crc16_ccitt_polynomial_ = 0x1021;

// Group for the command manager?


// Group for the sensor manager?


#endif // CONFIG_H
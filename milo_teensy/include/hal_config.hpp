#pragma once

#include <cstdint>

namespace MiloHal {
namespace Config {

// Serial protocol
constexpr uint16_t SYNC_BYTES = 0xAA55;
constexpr uint32_t BAUD_RATE = 1000000;
constexpr uint16_t CRC16_CCITT_INIT = 0xFFFF;
constexpr uint16_t CRC16_CCITT_POLYNOMIAL = 0x1021;

// Timing
constexpr uint32_t CONTROL_LOOP_HZ = 100;
constexpr uint32_t WATCHDOG_TIMEOUT_MS = 500;

// Pin defs
namespace Pins {
    // Drive motors

    // Front left
    constexpr uint8_t MOTOR_FL_PWM; // PWM
    constexpr uint8_t MOTOR_FL_DIR; // Direction
    constexpr uint8_t MOTOR_FL_SLP; // Sleep
    constexpr uint8_t MOTOR_FL_FLT; // Fault
    // constexpr uint8_t MOTOR_FL_CS; // Current sense
    constexpr uint8_t MOTOR_FL_ENC_A; // Encoder A
    constexpr uint8_t MOTOR_FL_ENC_B; // Encoder B

    // Middle left
    constexpr uint8_t MOTOR_FL_PWM;
    constexpr uint8_t MOTOR_FL_DIR;
    constexpr uint8_t MOTOR_FL_SLP;
    constexpr uint8_t MOTOR_FL_FLT;
    // constexpr uint8_t MOTOR_FL_CS;
    constexpr uint8_t MOTOR_FL_ENC_A;
    constexpr uint8_t MOTOR_FL_ENC_B;

    // Rear left
    constexpr uint8_t MOTOR_FL_PWM;
    constexpr uint8_t MOTOR_FL_DIR;
    constexpr uint8_t MOTOR_FL_SLP;
    constexpr uint8_t MOTOR_FL_FLT;
    // constexpr uint8_t MOTOR_FL_CS;
    constexpr uint8_t MOTOR_FL_ENC_A;
    constexpr uint8_t MOTOR_FL_ENC_B;

    // Front right
    constexpr uint8_t MOTOR_FL_PWM;
    constexpr uint8_t MOTOR_FL_DIR;
    constexpr uint8_t MOTOR_FL_SLP;
    constexpr uint8_t MOTOR_FL_FLT;
    // constexpr uint8_t MOTOR_FL_CS;
    constexpr uint8_t MOTOR_FL_ENC_A;
    constexpr uint8_t MOTOR_FL_ENC_B;

    // Middle right
    constexpr uint8_t MOTOR_FL_PWM;
    constexpr uint8_t MOTOR_FL_DIR;
    constexpr uint8_t MOTOR_FL_SLP;
    constexpr uint8_t MOTOR_FL_FLT;
    // constexpr uint8_t MOTOR_FL_CS;
    constexpr uint8_t MOTOR_FL_ENC_A;
    constexpr uint8_t MOTOR_FL_ENC_B;

    // Rear right
    constexpr uint8_t MOTOR_FL_PWM;
    constexpr uint8_t MOTOR_FL_DIR;
    constexpr uint8_t MOTOR_FL_SLP;
    constexpr uint8_t MOTOR_FL_FLT;
    // constexpr uint8_t MOTOR_FL_CS;
    constexpr uint8_t MOTOR_FL_ENC_A;
    constexpr uint8_t MOTOR_FL_ENC_B;

    // Steer servos
    constexpr uint8_t STEER_FL;
    constexpr uint8_t STEER_RL;
    constexpr uint8_t STEER_FR;
    constexpr uint8_t STEER_RR;

    // Auxiliary
    constexpr uint8_t HEADLIGHTS;
    constexpr uint8_t LED_STRIP;

}

} // namespace Config
} // namespace MiloHal
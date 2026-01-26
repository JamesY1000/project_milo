#ifndef BOARDCONFIG_H
#define BOARDCONFIG_H

#include <Arduino.h>

// DC motor pins
namespace Motor1
{
  constexpr uint8_t PWM = 6;    // PWM
  constexpr uint8_t DIR = 22;   // Direction
  constexpr uint8_t SLP = 28;   // Sleep
  constexpr uint8_t FLT = 31;   // Fault
  constexpr uint8_t CS  = A0;   // Current sense
  constexpr uint8_t ENC_A = 14; // Encoder A
  constexpr uint8_t ENC_B = 15; // Encoder B
}
// namespace Motor2 {
//   constexpr uint8_t PWM = 3;
//   constexpr uint8_t DIR = 23;
//   constexpr uint8_t SLP = 29;
//   constexpr uint8_t FLT = 32;
//   constexpr uint8_t CS  = A1;
//   constexpr uint8_t ENC_A = 16;
//   constexpr uint8_t ENC_B = 17;
// }

// Control loop timing
namespace Timing
{
  constexpr uint32_t CONTROL_PERIOD_MS   = 5;    // 200 Hz
  constexpr uint32_t WATCHDOG_TIMEOUT_MS = 250;  // Safety timeout
  constexpr uint32_t TELEMETRY_MS        = 100;  // 10 Hz
}

#endif // BOARDCONFIG_H
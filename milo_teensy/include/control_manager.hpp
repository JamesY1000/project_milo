#ifndef CONTROL_MANAGER_H
#define CONTROL_MANAGER_H

#include <Arduino.h>
#include "RoverCommand.pb.h"
#include "DualG2HighPowerMotorShield.h"

// Stores new RoverCommand message into local states
// Handles the commands sent from the SBC (drive/steer motors, aux commands, etc.)
class ControlManager
{
public:
    ControlManager();
    void begin();
    void applyRoverCommand(const RoverCommand& latest_cmd); // Applies commands inside RoverCommand, returns true if all commands successfully applied

private:
    // Each of these commands should have their own HAL/driver

    // TODO (james): Pass only required RoverCommand fields rather than the entire RoverCommand message
    void applyEStop(); // Highest priority, should override any other command
    void applyDriveCommands(const RoverCommand& latest_cmd);
    void applySteerCommands(const RoverCommand& latest_cmd);
    void applyAuxCommands(const RoverCommand& latest_cmd);
    void commandZeroMotorOutputs();

    void killAllMotors();
    void killAllAux();

    DualG2HighPowerMotorShield24v14 driver1_;
    DualG2HighPowerMotorShield18v18 driver2_;
    DualG2HighPowerMotorShield18v18 driver3_;

    bool headlights_on_;
    bool led_strip_on_;

    bool drivers_enabled_;
};

#endif // CONTROL_MANAGER_H
#ifndef COMMAND_MANAGER_H
#define COMMAND_MANAGER_H

#include <Arduino.h>
#include "RoverCommand.pb.h"

// Stores new RoverCommand message into local states
// Handles the commands sent from the SBC (drive/steer motors, aux commands, etc.)
class CommandManager
{
public:
    void applyRoverCommand(const RoverCommand& latest_cmd); // Applies commands inside RoverCommand, returns true if all commands successfully applied

private:
    // Each of these commands should have their own HAL/driver

    // TODO (james): Pass only required RoverCommand fields
    void applyEStop(const RoverCommand& latest_cmd); // Highest priority, should override any other command
    void applyDriveCommands(const RoverCommand& latest_cmd);
    void applySteerCommands(const RoverCommand& latest_cmd);
    void applyAuxCommands(const RoverCommand& latest_cmd);

    void killAllMotors();
    void killAllAux();

    bool headlights_on_{false};
    bool headlights_on_{false};
};

#endif // COMMAND_MANAGER_H
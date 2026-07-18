#include "command_manager.hpp"

void CommandManager::applyRoverCommand(const RoverCommand& latest_cmd)
{
    // Kill everything if Estop signal is sent
    if (latest_cmd.estop)
    {
        applyEStop(latest_cmd);
        return;
    }

    applyDriveCommands(latest_cmd);
    applySteerCommands(latest_cmd);
    applyAuxCommands(latest_cmd);

}

// Currently only sent if ros2 messages become stale
void CommandManager::applyEStop(const RoverCommand& latest_cmd)
{
    if (!latest_cmd.estop)
    {
        return;
    }
    killAllMotors();
    killAllAux();
    // killAllSensors(); - only if possible
    return;
}

void CommandManager::applyDriveCommands(const RoverCommand& latest_cmd)
{
    if (!latest_cmd.enable_motors)
    {
        return;
    }
    // ... Logic here for sending out drive commands
}

void CommandManager::applySteerCommands(const RoverCommand& latest_cmd)
{
    if (!latest_cmd.enable_motors)
    {
        return;
    }
    // ... Logic here for sending out steer commands
}

void CommandManager::applyAuxCommands(const RoverCommand& latest_cmd)
{
    // Check current headlight and led states with latest_cmd
    // Do not rewrite/reactivate to avoid redundancy - it should just be a toggle anyways no?

}

void CommandManager::killAllMotors()
{
    // Semd a 0 command to all motors
}

void CommandManager::killAllAux()
{
    // Set headlights to off
    // Set led strip to off
}
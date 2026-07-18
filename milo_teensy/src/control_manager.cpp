#include "control_manager.hpp"

void ControlManager::applyRoverCommand(const RoverCommand& latest_cmd)
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
void ControlManager::applyEStop(const RoverCommand& latest_cmd)
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

void ControlManager::applyDriveCommands(const RoverCommand& latest_cmd)
{
    if (!latest_cmd.enable_motors)
    {
        return;
    }
    // ... Logic here for sending out drive commands
}

void ControlManager::applySteerCommands(const RoverCommand& latest_cmd)
{
    if (!latest_cmd.enable_motors)
    {
        return;
    }
    // ... Logic here for sending out steer commands
}

void ControlManager::applyAuxCommands(const RoverCommand& latest_cmd)
{
    // Check current headlight and led states with latest_cmd
    // Do not rewrite/reactivate to avoid redundancy - it should just be a toggle anyways no?

}

void ControlManager::killAllMotors()
{
    // Semd a 0 command to all motors
}

void ControlManager::killAllAux()
{
    // Set headlights to off
    // Set led strip to off
}
#include "control_manager.hpp"

ControlManager::ControlManager()
{
    headlights_on_ = false;
    led_strip_on_ = false;
    drivers_enabled_ = false;
}

void ControlManager::begin()
{
    driver1_.init();
    driver2_.init();
    driver3_.init();

    driver1_.enableDrivers();
    driver2_.enableDrivers();
    driver3_.enableDrivers();
    drivers_enabled_ = true;
}


void ControlManager::applyRoverCommand(const RoverCommand& latest_cmd)
{
    // Check estop - kill everything if Estop signal is sent
    if (latest_cmd.estop)
    {
        applyEStop();
        return;
    }

    // If enable_motors flag is not received, stop rover motion but still apply aux commands
    if (!latest_cmd.enable_motors)
    {
        // Keep drivers enabled but force no motion
        commandZeroMotorOutputs();
        applyAuxCommands(latest_cmd);
        return;
    }

    if (!drivers_enabled_)
    {
        driver1_.enableDrivers();
        driver2_.enableDrivers();
        driver3_.enableDrivers();
        drivers_enabled_ = true;
        // delay(1); // Only delay if necessary
    }

    applyDriveCommands(latest_cmd);
    applySteerCommands(latest_cmd);
    applyAuxCommands(latest_cmd);
}

// Currently only sent if ros2 messages become stale
void ControlManager::applyEStop()
{
    killAllMotors();
    killAllAux();
    // killAllSensors(); - only if possible
    return;
}

void ControlManager::applyDriveCommands(const RoverCommand& latest_cmd)
{

    // ... Logic here for sending out drive commands
}

void ControlManager::applySteerCommands(const RoverCommand& latest_cmd)
{

    // ... Logic here for sending out steer commands
}

void ControlManager::applyAuxCommands(const RoverCommand& latest_cmd)
{
    // Check current headlight and led states with latest_cmd
    // Do not rewrite/reactivate to avoid redundancy - it should just be a toggle anyways no?

}

void ControlManager::killAllMotors()
{
    // Send a 0 command to motors, then disable all drivers
    commandZeroMotorOutputs();
    driver1_.disableDrivers();
    driver2_.disableDrivers();
    driver3_.disableDrivers();
    drivers_enabled_ = false;
}

void ControlManager::killAllAux()
{
    // Set headlights to off
    // Set led strip to off
}

void ControlManager::commandZeroMotorOutputs()
{
    // Send 0 to all motors
}
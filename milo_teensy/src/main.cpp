#include "serial_manager.hpp"
#include "RoverCommand.pb.h"

// Order of responsibilities:

// Execute SerialManager - decode RoverCommand.proto messages and store in local variables

// Execute CommandManager - executes motor commands for wheel dc motors, steering servo motors, and auxiliary systems

// Execute SensorManager - polls and returns current sensor data

// Execute SerialManager - encode RoverStatus.proto messages and serialWrite back to SBC


SerialManager serial_manager;
RoverCommand latest_cmd = RoverCommand_init_zero;



void loop()
{

    // Process incoming serial stream and consume (return) new RoverCommand if available
    serial_manager.updateSerial();

    if (serial_manager.consumeLatestCommand)
    {
        // Store local_cmd in local targets - pass through latest_cmd to motor_manager
    }

}
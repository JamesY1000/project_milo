#include "serial_manager.hpp"
#include "control_manager.hpp"
#include "RoverCommand.pb.h"

SerialManager serial_manager;
ControlManager control_manager;
// Sensor manager>

RoverCommand latest_cmd = RoverCommand_init_zero;


void loop()
{

    // Process incoming serial stream and consume (return) new RoverCommand if available
    serial_manager.updateSerial();

    if (serial_manager.consumeLatestCommand(latest_cmd))
    {
        // Pass latest command to command manager
        control_manager.applyRoverCommand(latest_cmd);
    }

}
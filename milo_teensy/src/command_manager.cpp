#include "command_manager.hpp"


// Create a class.called CommandManager

// Deserialise RoverCommand.proto messages

// Execute -> SerialManager - deserialise data and store into local data fields

// Execute -> DriveManager - executes motor commands for wheel dc motors and steering servo motors

// Execute -> AuxiliaryManager - turns lights on/off

// Execute sensor manager (poll sensors) - returns current sensor data

// Serialise sensor data

// Write sensor data back to SBC
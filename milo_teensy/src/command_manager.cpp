#include "command_manager.hpp"

// Create a class.called CommandManager
// This class handles motor and auxiliary commands from the SBC

// It should take in a RoverCommand message, store it into local states, then apply
// the commands for the required motors or applications (eg. auxiliary, estop)
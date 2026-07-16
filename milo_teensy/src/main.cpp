

// Order of responsibilities:

// Execute SerialManager - decode RoverCommand.proto messages and store in local variables

// Execute CommandManager - executes motor commands for wheel dc motors, steering servo motors, and auxiliary systems

// Execute SensorManager - polls and returns current sensor data

// Execute SerialManager - encode RoverStatus.proto messages and serialWrite back to SBC
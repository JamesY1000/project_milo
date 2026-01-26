#include <Arduino.h>

struct RoverCommand {
  float throttle      = 0.0f; // Throttle value
  float steer         = 0.0f; // Steering value
  bool enable         = false; // Arming switch
  uint32_t last_rx_ms = 0; // Latest received time (ms)
};

static RoverCommand latest_cmd;

static constexpr uint32_t CONTROL_PERIOD_MS   = 5; // 200 Hz - Control rate to send commands to motor
static constexpr uint32_t WATCHDOG_TIMEOUT_MS = 250; // 250 ms - Stops motors when no command is received after this time
static constexpr uint32_t TELEMETRY_MS        = 100; // 10 Hz - Rate at which telemetry is reported via serial
static constexpr uint32_t PWM_HZ              = 20000; // 20 kHz - Rate at which PWM signal is sent

static constexpr uint32_t SERIAL_BAUD = 115200;

static bool parseCmdLine(const String& line, RoverCommand& out)
{
  float throttle = 0.0f;
  float steer = 0.0f;
  int enable = 0;

  if (!line.startsWith("CMD ")) return false;
  
  if (sscanf(line.c_str(), "CMD %f %f %d", &throttle, &steer, &enable) != 3) return false;

  if (throttle > 1.0f) throttle = 1.0f;
  if (throttle < -1.0f) throttle = -1.0f;
  if (steer > 1.0f) steer = 1.0f;
  if (steer < -1.0f) steer = -1.0f;


  out.throttle = throttle;
  out.steer = steer;
  out.enable = (enable != 0);
  out.last_rx_ms = millis();
  return true;
}

static void applyOutputs(bool armed, const RoverCommand& cmd)
{
  if (!armed) {
    // TODO: SLP LOW, PWM = 0
    return;
  }

  // TODO: compute Ackermann and motor PWM here (armed case)
  // Armed case:
  // TODO: set SLP HIGH
  // TODO: DIR = sign(throttle), PWM = abs(throttle)
  // TODO: compute steering targets from cmd.steer

}


// TODO: Add all variables into header file
// TODO: Use sleep pin to stop the motor

// If a rovercommand is received within watchdog timeout, return true
static bool isArmed(const RoverCommand& rover_cmd)
{
  // Stop motors if no command received or watchdog limit exceeded
  if (!rover_cmd.enable) return false;

  const uint32_t now = millis();
  const uint32_t msg_gap_ms = now - rover_cmd.last_rx_ms;  // Current time - last received time
  if (msg_gap_ms > WATCHDOG_TIMEOUT_MS) return false;

  return true;
}

void setup() {
  // Initialise and setup the motors (6 DC, 4 Servos)
  // Initialise serial monitor
  // Write some setup ready message

  Serial.begin(SERIAL_BAUD);
  while (!Serial && millis() < 2000) {} // Give an initial delay before starting

  latest_cmd.enable = false;
  latest_cmd.last_rx_ms = millis();

  Serial.println("Milo teensy ready");
  Serial.println("Send: CMD <throttle -1 to 1> <steer -1 to 1> <enable 0/1>");
}

void loop() {

  // 1. Read/parse inputs (use serial for now)
  // 2. Control - Every control_period_ms tick, apply outputs if armed (stop all motors if not)
  // 3. Telemtry - Every telemetry_ms tick, print current state


  static uint32_t last_control_ms = 0;
  static uint32_t last_telem_ms = 0;
  static String rx_line;

  // Read serial, parse lines
  while (Serial.available())
  {
    char c = (char)Serial.read();
    
    if (c == '\n')
    {
      rx_line.trim();
      RoverCommand tmp = latest_cmd;
      
      if (parseCmdLine(rx_line, tmp))
      {
        latest_cmd = tmp;
        Serial.print("ACK ");
        Serial.print(latest_cmd.throttle, 3);
        Serial.print(" ");
        Serial.print(latest_cmd.steer, 3);
        Serial.print(" ");
        Serial.println(latest_cmd.enable ? 1 : 0); // Prints 1 if enable true, 0 if enable false
      }
      
      else if (rx_line.length() > 0)
      {
        Serial.print("ERR bad cmd: ");
        Serial.println(rx_line);
      }

      rx_line = "";
    } 
    
    else
    {
      rx_line += c;
    }
  }

  const uint32_t now = millis();

  // Control tick (200 Hz)
  if (now - last_control_ms >= CONTROL_PERIOD_MS)
  {
    last_control_ms = now;
    const bool armed = isArmed(latest_cmd);
    applyOutputs(armed, latest_cmd);
  }

  //  Telemtry tick (10 Hz)
  if (now - last_telem_ms >= TELEMETRY_MS)
  {
    last_telem_ms = now;
    const bool armed = isArmed(latest_cmd);

    Serial.print("STATE armed=");
    Serial.print(armed ? 1 : 0);
    Serial.print(" thr=");
    Serial.print(latest_cmd.throttle, 3);
    Serial.print(" str=");
    Serial.print(latest_cmd.steer, 3);
    Serial.print(" age_ms=");
    Serial.println(now - latest_cmd.last_rx_ms);
  }
}

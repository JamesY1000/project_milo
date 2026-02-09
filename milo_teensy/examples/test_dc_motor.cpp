#include "DualG2HighPowerMotorShield.h"
#include "BoardConfig.h"

DualG2HighPowerMotorShield24v14 md(
  Motor1::SLP,  // M1nSLEEP
  Motor1::DIR,  // M1DIR
  Motor1::PWM,  // M1PWM
  Motor1::FLT,  // M1nFAULT
  Motor1::CS,   // M1CS
  4,            // M2nSLEEP (dummy, not used)
  8,            // M2DIR (dummy)
  10,           // M2PWM (dummy)
  12,           // M2nFAULT (dummy)
  A1            // M2CS (dummy)
);


void stopIfFault()
{
  if (md.getM1Fault())
  {
    md.disableDrivers();
	  delay(1);
    Serial.println("M1 fault");
    while (1);
  }
  if (md.getM2Fault())
  {
    md.disableDrivers();
	  delay(1);
    Serial.println("M2 fault");
    while (1);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Simple Motor Test");
  md.init();
  md.calibrateCurrentOffsets();
  delay(10);
}

void loop()
{
  // Drive forward
  Serial.println("Forward");
  md.enableDrivers();
  delay(1);
  md.setM1Speed(400);
  delay(2000);
  
  // Stop
  Serial.println("Stop");
  md.setM1Speed(0);
  md.disableDrivers();
  delay(2000);

  // Drive backwards
  Serial.println("Backward");
  md.enableDrivers();
  delay(1);
  md.setM1Speed(-400);
  delay(2000);

  // Stop
  Serial.println("Stop");
  md.setM1Speed(0);
  md.disableDrivers();
  delay(2000);
}
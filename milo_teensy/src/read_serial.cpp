#include <Arduino.h>

void setup()
{
  Serial.begin(115200);
  while (!Serial) ; // Wait for Serial to be ready (optional, but useful)
  Serial.println("Teensy USB serial debug ready");
}

void loop()
{
  if (Serial.available())
  {
    String data = Serial.readStringUntil('\n');
    Serial.print("Received: ");
    Serial.println(data);
  }
}
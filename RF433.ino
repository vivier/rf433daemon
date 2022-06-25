/* https://github.com/sui77/rc-switch/ */

#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.begin(9600, SERIAL_8N1);
  mySwitch.enableReceive(0);
}

void loop() {
  uint32_t value;
  static uint32_t previous;
  static unsigned long last;

  if (!mySwitch.available()) {
    return;
  }

  value = mySwitch.getReceivedValue();
  mySwitch.resetAvailable();

  if ((value >> 8) == 0 || (value & 0xff) == 0) {
    return;
  }

  if (previous == value && millis() - last < 2000) {
    /* ignore duplicate events */
    return;
  }

  previous = value;
  last = millis();

  Serial.println(value, HEX);}

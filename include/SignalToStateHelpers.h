#ifndef _SIGNALTOSTATEHELPERS_H_
#define _SIGNALTOSTATEHELPERS_H_
#include <Arduino.h>

// Signal to state functions

// 1 == positive pressure (blow)
// 0 == no input detected
// -1 == negative pressure (suck)
int pressureSignalToState(int signal)
{
  if (signal > 12582912)
  {
    return 1;
  }
  else if (signal < 4194304)
  {
    return -1;
  }
  return 0;
}

int joystickSignalToState(int signal)
{
  if (signal > 3072)
  {
    return 1;
  }
  else if(signal < 2048)
  {
    return -1;
  }
  else return 0;
}

int buttonSignalToState(int signal)
{
  if(signal == 0)
  {
    return 1;
  }
  return 0;
}

// Read 24 bit input from pressure sensor
long readPressurePin(int pin, int sckOut)
{
  while (digitalRead(pin)) {}

  // read 24 bits
  long result = 0;
  for (int i = 0; i < 24; i++) {
    digitalWrite(sckOut, HIGH);
    digitalWrite(sckOut, LOW);
    result = result << 1;
    if (digitalRead(pin)) {
      result++;
    }
  }

  // get the 2s compliment
  result = result ^ 0x800000;
  return result;
}

enum DeviceModes
{
    Gamepad = 0,
    Keyboard = 1
};

#endif /* _SIGNALTOSTATEHELPERS_H_ */
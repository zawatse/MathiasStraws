#include "KeyPressStorage.h"
#include <Arduino.h>
#include "BLEDevice.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"
#include "BLEKeyboardHelpers.h"

int KeyPressStorage::getMask(const char key, unsigned long long &mask)
{
  uint8_t val = (uint8_t)key;
  if (val > KEYMAP_SIZE)
  {
    Serial.println("UNKNOWN CHAR");
    return -1; // character not available on keyboard - skip
  }
  mask = 1; 
  if(val > 127)
  {
    val -= 128;
    mask <<= val;
    return 2;
  }
  else if(val > 63)
  {
    val -= 64;
    mask <<= val;
    return 1;
  }
  else
  {
    mask <<= val;
    return 0;
  }
}

void KeyPressStorage::pressKey(const char key)
{
  unsigned long long mask = 0;
  switch(getMask(key, mask))
  {
    case 0:
      _map0 |= mask;
      break;
    case 1:
      _map1 |= mask;
      break;
    case 2:
      _map2 |= mask;
      break;
    default:
      Serial.println("UNKNOWN CHAR PRESS");
  };
}

void KeyPressStorage::releaseKey(const char key)
{
  unsigned long long mask = 0;
  switch(getMask(key, mask))
  {
    case 0:
      _map0 &= ~mask;
      break;
    case 1:
      _map1 &= ~mask;
      break;
    case 2:
      _map2 &= ~mask;
      break;
    default:
      Serial.println("UNKNOWN CHAR RELEASE");
  };
}

void KeyPressStorage::sendPresses()
{
  if ((*_input) == nullptr)
  {
    Serial.println("_input not yet initialied, not sending press");
    return;
  }
  uint8_t chars[6] = {};
  uint8_t modifiers = 0;
  int pressCounter = 0;
  // populate our KEYMAP array with buttons 
  unsigned long long temp0 = _map0;
  unsigned long long temp1 = _map1;
  unsigned long long temp2 = _map2;
  for (int i = 0; (i < 64) && (pressCounter < 6); i++)
  {
    if(temp0 & 1)
    {
      modifiers |= keymap[i].modifier;
      chars[pressCounter] = keymap[i].usage;
      pressCounter++;
    }
    if((temp1 & 1) && (pressCounter < 6))
    {
      modifiers |= keymap[i+64].modifier;
      chars[pressCounter] = keymap[i+64].usage;
      pressCounter++;
    }
    if((temp2 & 1) && (pressCounter < 6))
    {
      modifiers |= keymap[i+128].modifier;
      chars[pressCounter] = keymap[i+128].usage;
      pressCounter++;
    }
    temp0 >>= 1;
    temp1 >>= 1;
    temp2 >>= 1;
  }
  // create input report
  InputReport report = {
    .modifiers = modifiers,
    .reserved = 0,
    .pressedKeys = {
      chars[0],
      chars[1],
      chars[2],
      chars[3],
      chars[4],
      chars[5]
    }
  };
  // send the input report
  (*_input)->setValue((uint8_t*)&report, sizeof(report));
  (*_input)->notify();
  //delay(5);
}
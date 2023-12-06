#ifndef _KEYPRESSSTORAGE_H_
#define _KEYPRESSSTORAGE_H_
#include "BLEKeyboardHelpers.h"

// We require extreme speed and efficiency when pressing, releasing, and sending keystrokes
// Therefore we are using a bitmap. The keymap that stores the hex representations
// of each keystroke is simply an array where the index is the casted uint8 value of 
// any given char. We can take advantage of this by storing these uint8 values into a bitmap,
// and using the bitmap to create the array of chars and modifiers to send
class KeyPressStorage
{
public:
  KeyPressStorage(BLECharacteristic** input) : _input(input) {}  

  // create a mask for the key and return the map to which the key belongs
  int getMask(const char key, unsigned long long &mask);

  // Sets bit to 1 on the correlating map for the key
  // using a bitwise OR
  void pressKey(const char key);

  // Sets bit to 0 on the correlating map for the key
  // using a bitwise AND with the not of the mask
  void releaseKey(const char key);

  // Send all currently saved presses
  void sendPresses();
  
private:
  unsigned long long _map0 = 0;
  unsigned long long _map1 = 0;
  unsigned long long _map2 = 0;
  BLECharacteristic** _input;
};

#endif /* _KEYPRESSSTORAGE_H_ */

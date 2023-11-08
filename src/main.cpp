#include <Arduino.h>
#define US_KEYBOARD
#include "BLEDevice.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"
#define DEVICE_NAME "Mathias Gamepad"
void typeText(const char* text);
bool isBleConnected = false;

// Define clock pin, used for pressure sensor
#define SCK_OUT               SCK // 18

// Define pressure sensor input pins
//#define PRESSURE_IN_1       T0 // 4

// Define joystick input pins
// These will be analog input pins reading potentiometer inputs along perpendicular axis, 
// along with one pin for the joystick button
#define JOYSTICK_X_IN_0       A0 // 36
#define JOYSTICK_Y_IN_0       A3 // 39
//#define JOYSTICK_BUTTON_IN_0  A6 // 34

// Define button input pins and keys
#define BUTTON_IN_0           GPIO_NUM_32
#define BUTTON_IN_1           GPIO_NUM_33
#define BUTTON_IN_2           GPIO_NUM_25
#define BUTTON_IN_3           GPIO_NUM_26
#define BUTTON_IN_4           GPIO_NUM_27
#define BUTTON_IN_5           GPIO_NUM_14

#define BUTTON_0_KEY          'a'
#define BUTTON_1_KEY          'e'
#define BUTTON_2_KEY          'i'
#define BUTTON_3_KEY          'o'
#define BUTTON_4_KEY          'u'
#define BUTTON_5_KEY          '!'


// Forward declarations
void bluetoothTask(void*);
int joystickSignalToState(int signal);
void typeText(const char* text);
void releaseAllKeys();

//
// This part of the program lets an ESP32 act as a keyboard connected via Bluetooth.
// When a button attached to the ESP32 is pressed, it will generate the key strokes for a message.
//

// Message (report) sent when a key is pressed or released
struct InputReport {
    uint8_t modifiers;        // bitmask: CTRL = 1, SHIFT = 2, ALT = 4
    uint8_t reserved;        // must be 0
    uint8_t pressedKeys[6];  // up to six concurrenlty pressed keys
};
// Message (report) received when an LED's state changed
struct OutputReport {
    uint8_t leds;            // bitmask: num lock = 1, caps lock = 2, scroll lock = 4, compose = 8, kana = 16
};

BLEHIDDevice* hid;
BLECharacteristic* input;
BLECharacteristic* output;
const InputReport NO_KEY_PRESSED = {};

// We require extreme speed and efficiency when pressing, releasing, and sending keystrokes
// Therefore we are using a bitmap. The keymap that stores the hex representations
// of each keystroke is simply an array where the index is the casted uint8 value of 
// any given char. We can take advantage of this by storing these uint8 values into a bitmap,
// and using the bitmap to create the array of chars and modifiers to send
class KeyPressStorage
{
public:
  // create a mask for the key and return the map to which the key belongs
  int getMask(const char key, unsigned long long &mask)
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

  // Sets bit to 1 on the correlating map for the key
  // using a bitwise OR
  void pressKey(const char key)
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

  // Sets bit to 0 on the correlating map for the key
  // using a bitwise AND with the not of the mask
  void releaseKey(const char key)
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

  void sendPresses()
  {
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
    input->setValue((uint8_t*)&report, sizeof(report));
    input->notify();
    //delay(5);
  }

private:
  unsigned long long _map0 = 0;
  unsigned long long _map1 = 0;
  unsigned long long _map2 = 0;
};

KeyPressStorage keyPresser;

// Binary State Machine for Button
class ButtonStateMachine
{
public:
  enum ButtonState
  {
    ON, // 0
    OFF // 1
  };

  ButtonStateMachine(const char key) : _state(ButtonState::OFF), _key(key) {}
  
  void UpdateState(ButtonState newState)
  {
    switch(newState)
    {
      case OFF:
        switch(_state)
        {
          case ON:
            // BUTTON OFF CODE
            keyPresser.releaseKey(_key);
            keyPresser.sendPresses();
            _state = OFF;
            break;

          default:
            break;
        }
        break;

      case ON:
        switch(_state)
        {
          case OFF:
            // BUTTON ON CODE
            keyPresser.pressKey(_key);
            keyPresser.sendPresses();
            _state = ON;
            break;

          default:
            break;
        }
        break;

      default:
        Serial.println("UNEXPECTED STATE");
        _state = OFF;
        break;  
    }
  }

private:
  ButtonState _state;
  const char _key;
};

// Create state machines for inputs
ButtonStateMachine _button0StateMachine(BUTTON_0_KEY);
ButtonStateMachine _button1StateMachine(BUTTON_1_KEY);
ButtonStateMachine _button2StateMachine(BUTTON_2_KEY);
ButtonStateMachine _button3StateMachine(BUTTON_3_KEY);
ButtonStateMachine _button4StateMachine(BUTTON_4_KEY);
ButtonStateMachine _button5StateMachine(BUTTON_5_KEY);

void setup() {
  // Pin Setup
  //
  // Pins are set to INPUT by default. {ins configured as pinMode(pin, INPUT) with nothing connected to them, 
  // or with wires connected to them that are not connected to other circuits, will report seemingly random changes 
  // in pin state, picking up electrical noise from the environment, or capacitively coupling the state of a nearby pin. 
  // 
  // INPUT_PULLUP configures the pin's internal pullup resistor to be activated, making the default value on the pin HIGH
  //pinMode(SCK_OUT, OUTPUT);
  //pinMode(PRESSURE_IN_1, INPUT);
  pinMode(JOYSTICK_X_IN_0, INPUT);
  pinMode(JOYSTICK_Y_IN_0, INPUT);
  //pinMode(JOYSTICK_BUTTON_IN_0, INPUT_PULLUP);
  pinMode(BUTTON_IN_0, INPUT_PULLUP);
  pinMode(BUTTON_IN_1, INPUT_PULLUP);
  pinMode(BUTTON_IN_2, INPUT_PULLUP);
  pinMode(BUTTON_IN_3, INPUT_PULLUP);
  pinMode(BUTTON_IN_4, INPUT_PULLUP);
  pinMode(BUTTON_IN_5, INPUT_PULLUP);

  // Create an Asynch task to initialize bluetooth
  xTaskCreate(bluetoothTask, "bluetooth", 20000, NULL, 5, NULL);

  // Initialize serial connection with low baud rate for debugging
  Serial.begin(9600);
}

// Main loop
// Purpose:
// Run the clock if needed
// Read input pins
// Map values to states
// Send bluetooth presses
// PERF TODO: Send presses Asynchronously
// PERF TODO: Create state machines for buttons 
void loop() {
  // Pulse the clock line 3 times to start the next pressure reading for pressure sensors
  //for (char i = 0; i < 3; i++) {
  //  digitalWrite(SCK_PIN, HIGH);
  //  digitalWrite(SCK_PIN, LOW);
  //}

  // Convert values to states
  int joyXState = joystickSignalToState(analogRead(JOYSTICK_X_IN_0));
  int joyYState = joystickSignalToState(analogRead(JOYSTICK_Y_IN_0));

  // Update State Machines, send chars
  _button0StateMachine.UpdateState(static_cast<ButtonStateMachine::ButtonState>(digitalRead(BUTTON_IN_0)));
  _button1StateMachine.UpdateState(static_cast<ButtonStateMachine::ButtonState>(digitalRead(BUTTON_IN_1)));
  _button2StateMachine.UpdateState(static_cast<ButtonStateMachine::ButtonState>(digitalRead(BUTTON_IN_2)));
  _button3StateMachine.UpdateState(static_cast<ButtonStateMachine::ButtonState>(digitalRead(BUTTON_IN_3)));
  _button4StateMachine.UpdateState(static_cast<ButtonStateMachine::ButtonState>(digitalRead(BUTTON_IN_4)));
  _button5StateMachine.UpdateState(static_cast<ButtonStateMachine::ButtonState>(digitalRead(BUTTON_IN_5)));

  // Printout for debugger
  //Serial.println(analogRead(JOYSTICK_X_IN_0));
  //Serial.println(analogRead(JOYSTICK_Y_IN_0));
  //Serial.println(digitalRead(BUTTON_IN_0));
  //Serial.println(digitalRead(BUTTON_IN_1));
  //Serial.println(digitalRead(BUTTON_IN_2));
  //Serial.println(digitalRead(BUTTON_IN_3));
  //Serial.println(digitalRead(BUTTON_IN_4));
  //Serial.println(digitalRead(BUTTON_IN_5));  
  //Serial.println("ENDENDENDEND");
  //delay(500);
}

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
  if (signal > 3074)
  {
    return 1;
  }
  else if(signal < 1024)
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
long readPressurePin(int pin)
{
  while (digitalRead(pin)) {}

  // read 24 bits
  long result = 0;
  for (int i = 0; i < 24; i++) {
    digitalWrite(SCK_OUT, HIGH);
    digitalWrite(SCK_OUT, LOW);
    result = result << 1;
    if (digitalRead(pin)) {
      result++;
    }
  }

  // get the 2s compliment
  result = result ^ 0x800000;
  return result;
}

/*
 * Sample program for ESP32 acting as a Bluetooth keyboard
 *
 * Copyright (c) 2019 Manuel Bl
 *
 * Licensed under MIT License
 * https://opensource.org/licenses/MIT
 */

// The report map describes the HID device (a keyboard in this case) and
// the messages (reports in HID terms) sent and received.
static const uint8_t REPORT_MAP[] = {
    USAGE_PAGE(1),      0x01,       // Generic Desktop Controls
    USAGE(1),           0x06,       // Keyboard
    COLLECTION(1),      0x01,       // Application
    REPORT_ID(1),       0x01,       //   Report ID (1)
    USAGE_PAGE(1),      0x07,       //   Keyboard/Keypad
    USAGE_MINIMUM(1),   0xE0,       //   Keyboard Left Control
    USAGE_MAXIMUM(1),   0xE7,       //   Keyboard Right Control
    LOGICAL_MINIMUM(1), 0x00,       //   Each bit is either 0 or 1
    LOGICAL_MAXIMUM(1), 0x01,
    REPORT_COUNT(1),    0x08,       //   8 bits for the modifier keys
    REPORT_SIZE(1),     0x01,      
    HIDINPUT(1),        0x02,       //   Data, Var, Abs
    REPORT_COUNT(1),    0x01,       //   1 byte (unused)
    REPORT_SIZE(1),     0x08,
    HIDINPUT(1),        0x01,       //   Const, Array, Abs
    REPORT_COUNT(1),    0x06,       //   6 bytes (for up to 6 concurrently pressed keys)
    REPORT_SIZE(1),     0x08,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x65,       //   101 keys
    USAGE_MINIMUM(1),   0x00,
    USAGE_MAXIMUM(1),   0x65,
    HIDINPUT(1),        0x00,       //   Data, Array, Abs
    REPORT_COUNT(1),    0x05,       //   5 bits (Num lock, Caps lock, Scroll lock, Compose, Kana)
    REPORT_SIZE(1),     0x01,
    USAGE_PAGE(1),      0x08,       //   LEDs
    USAGE_MINIMUM(1),   0x01,       //   Num Lock
    USAGE_MAXIMUM(1),   0x05,       //   Kana
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x01,
    HIDOUTPUT(1),       0x02,       //   Data, Var, Abs
    REPORT_COUNT(1),    0x01,       //   3 bits (Padding)
    REPORT_SIZE(1),     0x03,
    HIDOUTPUT(1),       0x01,       //   Const, Array, Abs
    END_COLLECTION(0)               // End application collection
};

/*
 * Callbacks related to BLE connection
 */
class BleKeyboardCallbacks : public BLEServerCallbacks{
    void onConnect(BLEServer* server) {
        isBleConnected = true;
        // Allow notifications for characteristics
        BLE2902* cccDesc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
        cccDesc->setNotifications(true);
        Serial.println("Client has connected");
    }
    void onDisconnect(BLEServer* server) {
        isBleConnected = false;
        // Disallow notifications for characteristics
        BLE2902* cccDesc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
        cccDesc->setNotifications(false);
        Serial.println("Client has disconnected");
    }
};
/*
 * Called when the client (computer, smart phone) wants to turn on or off
 * the LEDs in the keyboard.
 *
 * bit 0 - NUM LOCK
 * bit 1 - CAPS LOCK
 * bit 2 - SCROLL LOCK
 */
 class OutputCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* characteristic) {
        OutputReport* report = (OutputReport*) characteristic->getData();
        //Serial.print("LED state: ");
        //Serial.print((int) report->leds);
        //Serial.println();
    }
 };

void bluetoothTask(void*){
    // initialize the device
    BLEDevice::init(DEVICE_NAME);
    BLEServer* server = BLEDevice::createServer();
    server->setCallbacks(new BleKeyboardCallbacks());
    // create an HID device
    hid = new BLEHIDDevice(server);
    input = hid->inputReport(1); // report ID
    output = hid->outputReport(1); // report ID
    output->setCallbacks(new OutputCallbacks());
    // set manufacturer name
    hid->manufacturer()->setValue("Maker Community");
    // set USB vendor and product ID
    hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
    // information about HID device: device is not localized, device can be connected
    hid->hidInfo(0x00, 0x02);
    // Security: device requires bonding
    BLESecurity* security = new BLESecurity();
    security->setAuthenticationMode(ESP_LE_AUTH_BOND);
    // set report map
    hid->reportMap((uint8_t*)REPORT_MAP, sizeof(REPORT_MAP));
    hid->startServices();
    // set battery level to 100%
    hid->setBatteryLevel(100);
    // advertise the services
    BLEAdvertising* advertising = server->getAdvertising();
    advertising->setAppearance(HID_KEYBOARD);
    advertising->addServiceUUID(hid->hidService()->getUUID());
    advertising->addServiceUUID(hid->deviceInfo()->getUUID());
    advertising->addServiceUUID(hid->batteryService()->getUUID());
    advertising->start();
    //Serial.println("BLE ready");
    delay(portMAX_DELAY);
}
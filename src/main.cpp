#include <Arduino.h>
#define SCK_PIN 18
#define STRAW_1_PIN 4
#define STRAW_2_PIN 2
#define STRAW_3_PIN 15
#define STRAW_4_PIN 14
#define JOYSTICK_X 36
#define JOYSTICK_Y 39
#define JOYSTICK_BUTTON 34


int signalToState(int signal)
{
  if (signal > 12582912)
  {
    return 2;
  }
  else if (signal < 4194304)
  {
    return 0;
  }
  return 1;
}

int joystickSignalToState(int signal)
{
  if (signal > 3074)
  {
    return 2;
  }
  else if(signal < 1024)
  {
    return 0;
  }
  else return 1;
}

int buttonSignalToState(int signal)
{
  if(signal == 0)
  {
    return 2;
  }
  return 1;
}

long readPin(int pin)
{
  while (digitalRead(pin)) {}

  // read 24 bits
  long result = 0;
  for (int i = 0; i < 24; i++) {
    digitalWrite(SCK_PIN, HIGH);
    digitalWrite(SCK_PIN, LOW);
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

//
// This program lets an ESP32 act as a keyboard connected via Bluetooth.
// When a button attached to the ESP32 is pressed, it will generate the key strokes for a message.
//
// For the setup, a momentary button should be connected to pin 2 and to ground.
// Pin 2 will be configured as an input with pull-up.
//
// In order to receive the message, add the ESP32 as a Bluetooth keyboard of your computer
// or mobile phone:
//
// 1. Go to your computers/phones settings
// 2. Ensure Bluetooth is turned on
// 3. Scan for Bluetooth devices
// 4. Connect to the device called "ESP32 Keyboard"
// 5. Open an empty document in a text editor
// 6. Press the button attached to the ESP32

#define US_KEYBOARD
#include <Arduino.h>
#include "BLEDevice.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"
// Change the below values if desired
#define BUTTON_PIN1 13
#define BUTTON_PIN2 12
#define BUTTON_PIN3 14
#define MESSAGE1 "A\n"
#define MESSAGE2 "B\n"
#define MESSAGE3 "C\n"
#define DEVICE_NAME "Connect to M"
// Forward declarations
void bluetoothTask(void*);
void typeText(const char* text);
bool isBleConnected = false;

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

BLEHIDDevice* hid;
BLECharacteristic* input;
BLECharacteristic* output;
const InputReport NO_KEY_PRESSED = {};
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
        Serial.print("LED state: ");
        Serial.print((int) report->leds);
        Serial.println();
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
    Serial.println("BLE ready");
    delay(portMAX_DELAY);
}

void unpressKey()
{
  input->setValue((uint8_t*)&NO_KEY_PRESSED, sizeof(NO_KEY_PRESSED));
  input->notify();
  delay(5);
}

void typeText(const char* text, bool unpress = true) {
    int len = strlen(text);
    for (int i = 0; i < len; i++){
        // translate character to key combination
        uint8_t val = (uint8_t)text[i];
        if (val > KEYMAP_SIZE)
            continue; // character not available on keyboard - skip
        KEYMAP map = keymap[val];
        // create input report
        InputReport report = {
            .modifiers = map.modifier,
            .reserved = 0,
            .pressedKeys = {
                map.usage,
                0, 0, 0, 0, 0
            }
        };
        
        // send the input report
        input->setValue((uint8_t*)&report, sizeof(report));
        input->notify();
        delay(5);

        if(unpress)
        {
          // release all keys between two characters; otherwise two identical
          // consecutive characters are treated as just one key press
          //input->setValue((uint8_t*)&NO_KEY_PRESSED, sizeof(NO_KEY_PRESSED));
          //input->notify();
          //delay(5);
          unpressKey();
        }
    }
}

int prev0 = 1;
int prev1 = 1;
int prev2 = 1;
int prev3 = 1;
int prevx = 1;
int prevy = 1;

void sendPresses(int in0, int in1, int in2, int in3, int joyInx, int joyIny)
{
  {
    switch(in0)
    {
      case 0:
        typeText("A",false);
        break;
      case 2:
        typeText("B",false);
        break;
      case 1:
        if(prev0 == 2 || prev0 == 0)
        {
          unpressKey();
        }
        break;
    }
  }
  {
    switch(in1)
    {
      case 0:
        typeText("C", false);
        break;
      case 2:
        typeText("D", false);
        break;
      case 1:
        if(prev1 == 2 || prev1 == 0)
        {
          unpressKey();
        }
        break;
    }
  }
  {
    switch(in2)
    {
      case 0:
        typeText("E", true);
        break;
      case 2:
        typeText("F", true);
        break;
    }
  }
  {
    switch(in3)
    {
      case 0:
        typeText("G", true);
        break;
      case 2:
        typeText("H", true);
        break;
    }
  }
  {
    switch(joyInx)
    {
      case 0:
        typeText("w",false);
        break;
      case 2:
        typeText("s",false);
        break;
      case 1:
        if(!(prevx == 1))
        {
          unpressKey();
        }
        break;
    }
  }
  {
    switch(joyIny)
    {
      case 0:
        typeText("d",false);
        break;
      case 2:
        typeText("a",false);
        break;
      case 1:
        if(!(prevy == 1))
        {
          unpressKey();
        }
        break;
    }
  }
  prev0 = in0;
  prev1 = in1;
  prev2 = in2;
  prev3 = in3;
  prevx = joyInx;
  prevy = joyIny;
}

void setup() {
  pinMode(STRAW_1_PIN, INPUT);   // Connect HX710 OUTs to Arduino pins 4, 5, 6, 7
  pinMode(STRAW_2_PIN, INPUT); 
  pinMode(STRAW_3_PIN, INPUT); 
  pinMode(STRAW_4_PIN, INPUT); 
  pinMode(SCK_PIN, OUTPUT);  // Connect HX710 SCK to Arduino pin 3
  xTaskCreate(bluetoothTask, "bluetooth", 20000, NULL, 5, NULL);
  Serial.begin(9600);
}

void loop() {
  long result1 = readPin(STRAW_1_PIN);
  long result2 = readPin(STRAW_2_PIN);
  long result3 = readPin(STRAW_3_PIN);
  long result4 = readPin(STRAW_4_PIN);

  // pulse the clock line 3 times to start the next pressure reading
  for (char i = 0; i < 3; i++) {
    digitalWrite(SCK_PIN, HIGH);
    digitalWrite(SCK_PIN, LOW);
  }

  int joyInx = joystickSignalToState(analogRead(JOYSTICK_X));
  int joyIny = joystickSignalToState(analogRead(JOYSTICK_Y));
  // display pressure
  //Serial.println(result);
  Serial.println(result1);
  Serial.println(signalToState(result2));
  Serial.println(signalToState(result3));
  Serial.println(signalToState(result4));
  Serial.println(joyInx);
  Serial.println(joyIny);
  Serial.println(buttonSignalToState(analogRead(JOYSTICK_BUTTON)));
  sendPresses(signalToState(result1), signalToState(result2), signalToState(result3), signalToState(result4), joyInx, joyIny );
  Serial.println("ENDENDENDEND");
  delay(500);
}
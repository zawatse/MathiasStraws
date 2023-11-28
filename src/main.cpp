#include <Arduino.h>
#include <BleGamepad.h>

#include "BLEGamepadHelpers.h"
#include "BLEKeyboardHelpers.h"
#include "KeyPressStorage.h"
#include "SignalToStateHelpers.h"
#include "StateMachines.h"

// Define pin to detemrine mode
// default will be gamepad
// alternate will be keyboard
#define MODE_SWITCH                 GPIO_NUM_4
DeviceModes _deviceMode = DeviceModes::Gamepad;
#define JOYSTICK_SWITCH_L           GPIO_NUM_18
#define JOYSTICK_SWITCH_R           GPIO_NUM_17
bool _joystickLOn = false;
bool _joystickROn = false;

//
// GAMEPAD DEFINES
//

// JOYSTICK
#define LH A0
#define LV A3
#define RH A6
#define RV A7

// // D-PAD
#define L1 GPIO_NUM_27
#define L2 GPIO_NUM_14
#define L3 GPIO_NUM_12
#define L4 GPIO_NUM_13

#define R1 GPIO_NUM_32
#define R2 GPIO_NUM_33
#define R3 GPIO_NUM_25
#define R4 GPIO_NUM_26

// TRIGGER BUTTONS
#define LT GPIO_NUM_23
#define LB GPIO_NUM_21
#define LS GPIO_NUM_1

#define RT GPIO_NUM_22
#define RB GPIO_NUM_19
#define RS GPIO_NUM_3

// Define joystick input pins and keys
// These will be analog input pins reading potentiometer inputs along perpendicular axis, 
// along with one pin for the joystick button
#define JOYSTICK_X_IN_0             LH
#define JOYSTICK_Y_IN_0             LV 
#define JOYSTICK_X_O_LEFT_KEY       static_cast<const char>(LEFT_ARROW)
#define JOYSTICK_X_O_RIGHT_KEY      static_cast<const char>(RIGHT_ARROW)
#define JOYSTICK_Y_O_UP_KEY         static_cast<const char>(UP_ARROW)
#define JOYSTICK_Y_O_DOWN_KEY       static_cast<const char>(DOWN_ARROW)

#define JOYSTICK_X_IN_1             RH
#define JOYSTICK_Y_IN_1             RV
#define JOYSTICK_X_1_LEFT_KEY       'a'
#define JOYSTICK_X_1_RIGHT_KEY      'd'
#define JOYSTICK_Y_1_UP_KEY         'w'
#define JOYSTICK_Y_1_DOWN_KEY       's'

//#define JOYSTICK_BUTTON_IN_0  A6 // 34

// Define button input pins and keys
#define BUTTON_IN_0           R1
#define BUTTON_IN_1           R2
#define BUTTON_IN_2           R3
#define BUTTON_IN_3           R4
#define BUTTON_IN_4           LT
#define BUTTON_IN_5           RT

#define BUTTON_0_KEY          'q' 
#define BUTTON_1_KEY          'e'
#define BUTTON_2_KEY          'r'
#define BUTTON_3_KEY          'f'
#define BUTTON_4_KEY          'c'
#define BUTTON_5_KEY          'x'

//
// GAMEPAD SETUP
//

// Create bluetooth helper objects for gamepad
BleGamepad _bleGamepad("MathiasController");

GamepadButtonStateMachine _gamepadButton0StateMachine(BUTTON_1, &_bleGamepad);
GamepadButtonStateMachine _gamepadButton1StateMachine(BUTTON_2, &_bleGamepad);
GamepadButtonStateMachine _gamepadButton2StateMachine(BUTTON_3, &_bleGamepad);
GamepadButtonStateMachine _gamepadButton3StateMachine(BUTTON_4, &_bleGamepad);
GamepadButtonStateMachine _gamepadButton4StateMachine(BUTTON_5, &_bleGamepad);
GamepadButtonStateMachine _gamepadButton5StateMachine(BUTTON_6, &_bleGamepad);
GamepadButtonStateMachine _gamepadButton6StateMachine(BUTTON_7, &_bleGamepad);
GamepadButtonStateMachine _gamepadButton7StateMachine(BUTTON_8, &_bleGamepad);
GamepadButtonStateMachine _gamepadButton8StateMachine(BUTTON_9, &_bleGamepad);
GamepadButtonStateMachine _gamepadButton9StateMachine(BUTTON_10, &_bleGamepad);
GamepadButtonStateMachine _gamepadButton10StateMachine(BUTTON_11, &_bleGamepad);
GamepadButtonStateMachine _gamepadButton11StateMachine(BUTTON_12, &_bleGamepad);
GamepadButtonStateMachine _gamepadButton12StateMachine(BUTTON_13, &_bleGamepad);
GamepadButtonStateMachine _gamepadButton13StateMachine(BUTTON_14, &_bleGamepad);

//
// KEYBOARD SETUP
//

// Create bluetooth helper objects for keypad
BLEHIDDevice *_hid;
BLECharacteristic *_input;
BLECharacteristic *_output;

//Create keyPresser
KeyPressStorage _keyPresser(&_input);

// Create state machines for inputs
ButtonStateMachine _button0StateMachine(BUTTON_0_KEY, &_keyPresser);
ButtonStateMachine _button1StateMachine(BUTTON_1_KEY, &_keyPresser);
ButtonStateMachine _button2StateMachine(BUTTON_2_KEY, &_keyPresser);
ButtonStateMachine _button3StateMachine(BUTTON_3_KEY, &_keyPresser);
ButtonStateMachine _button4StateMachine(BUTTON_4_KEY, &_keyPresser);
ButtonStateMachine _button5StateMachine(BUTTON_5_KEY, &_keyPresser);

JoystickStateMachine _joystick0XStateMachine(JOYSTICK_X_O_LEFT_KEY, JOYSTICK_X_O_RIGHT_KEY, &_keyPresser);
JoystickStateMachine _joystick0YStateMachine(JOYSTICK_Y_O_UP_KEY, JOYSTICK_Y_O_DOWN_KEY, &_keyPresser);

JoystickStateMachine _joystick1XStateMachine(JOYSTICK_X_1_LEFT_KEY, JOYSTICK_X_1_RIGHT_KEY, &_keyPresser);
JoystickStateMachine _joystick1YStateMachine(JOYSTICK_Y_1_UP_KEY, JOYSTICK_Y_1_DOWN_KEY, &_keyPresser);

void bluetoothTask(void*)
{
  bluetoothKeyboardSetup(&_hid, &_input, &_output);
}


void setup() {
  // Pin Setup
  //
  // Pins are set to INPUT by default. {ins configured as pinMode(pin, INPUT) with nothing connected to them, 
  // or with wires connected to them that are not connected to other circuits, will report seemingly random changes 
  // in pin state, picking up electrical noise from the environment, or capacitively coupling the state of a nearby pin. 
  // 
  // INPUT_PULLUP configures the pin's internal pullup resistor to be activated, making the default value on the pin HIGH
  pinMode(MODE_SWITCH, INPUT_PULLUP);
  pinMode(JOYSTICK_SWITCH_L, INPUT_PULLUP);
  pinMode(JOYSTICK_SWITCH_R, INPUT_PULLUP);

  pinMode(R1, INPUT_PULLUP);
  pinMode(R2, INPUT_PULLUP);
  pinMode(R3, INPUT_PULLUP);
  pinMode(R4, INPUT_PULLUP);
  pinMode(L1, INPUT_PULLUP);
  pinMode(L2, INPUT_PULLUP);
  pinMode(L3, INPUT_PULLUP);
  pinMode(L4, INPUT_PULLUP);
  pinMode(RT, INPUT_PULLUP);
  pinMode(RB, INPUT_PULLUP);
  pinMode(RS, INPUT_PULLUP);
  pinMode(LT, INPUT_PULLUP);
  pinMode(LB, INPUT_PULLUP);
  pinMode(LS, INPUT_PULLUP);

  pinMode(LH, INPUT);
  pinMode(LV, INPUT);
  pinMode(RH, INPUT);
  pinMode(RV, INPUT);

  // Detect joystick states
  // Give time for pullups to turn on
  // We want default to be off
  delay(10);
  _joystickLOn = digitalRead(JOYSTICK_SWITCH_L) == 0;
  _joystickROn = digitalRead(JOYSTICK_SWITCH_R) == 0;
  switch(digitalRead(MODE_SWITCH))
  {
    case 0:
      xTaskCreate(bluetoothTask, "bluetooth", 20000, NULL, 5, NULL);
      _deviceMode = DeviceModes::Keyboard;
      break;
    case 1:
    default:
      _deviceMode = DeviceModes::Gamepad;
      _bleGamepad.begin();
      break;
  }

  // Initialize serial connection with low baud rate for debugging
  Serial.begin(9600);
}

void GamepadOperation()
{
    if (_bleGamepad.isConnected()) {
      _gamepadButton0StateMachine.UpdateState(static_cast<GamepadButtonStateMachine::ButtonState>(digitalRead(R1)));
      _gamepadButton1StateMachine.UpdateState(static_cast<GamepadButtonStateMachine::ButtonState>(digitalRead(R2)));
      _gamepadButton2StateMachine.UpdateState(static_cast<GamepadButtonStateMachine::ButtonState>(digitalRead(R3)));
      _gamepadButton3StateMachine.UpdateState(static_cast<GamepadButtonStateMachine::ButtonState>(digitalRead(R4)));
      _gamepadButton4StateMachine.UpdateState(static_cast<GamepadButtonStateMachine::ButtonState>(digitalRead(L1)));
      _gamepadButton5StateMachine.UpdateState(static_cast<GamepadButtonStateMachine::ButtonState>(digitalRead(L2)));
      _gamepadButton6StateMachine.UpdateState(static_cast<GamepadButtonStateMachine::ButtonState>(digitalRead(L3)));
      _gamepadButton7StateMachine.UpdateState(static_cast<GamepadButtonStateMachine::ButtonState>(digitalRead(L4)));
      _gamepadButton8StateMachine.UpdateState(static_cast<GamepadButtonStateMachine::ButtonState>(digitalRead(LT)));
      _gamepadButton9StateMachine.UpdateState(static_cast<GamepadButtonStateMachine::ButtonState>(digitalRead(RT)));
      _gamepadButton10StateMachine.UpdateState(static_cast<GamepadButtonStateMachine::ButtonState>(digitalRead(LB)));
      _gamepadButton11StateMachine.UpdateState(static_cast<GamepadButtonStateMachine::ButtonState>(digitalRead(RB)));
      //_gamepadButton12StateMachine.UpdateState(static_cast<GamepadButtonStateMachine::ButtonState>(digitalRead(LS)));
      //_gamepadButton13StateMachine.UpdateState(static_cast<GamepadButtonStateMachine::ButtonState>(digitalRead(RS)));

      ProcessJoystickVals(_bleGamepad, RH, RV, LH, LV, _joystickROn, _joystickLOn);
    }
  //Serial.println("GAMEPAD");
  //delay(1000);
}

void KeyboardOperation()
{
  // Convert values to states
  switch(_joystickLOn)
  {
    case true:
      _joystick0XStateMachine.UpdateState(static_cast<JoystickStateMachine::JoystickState>(joystickSignalToState(analogRead(JOYSTICK_X_IN_0))));
      _joystick0YStateMachine.UpdateState(static_cast<JoystickStateMachine::JoystickState>(joystickSignalToState(analogRead(JOYSTICK_Y_IN_0))));
      //Serial.println(analogRead(JOYSTICK_X_IN_0));
      //Serial.println(analogRead(JOYSTICK_Y_IN_0));
      break;
    default:
      break;
  }
  
  switch(_joystickROn)
  {
    case true:
      _joystick1XStateMachine.UpdateState(static_cast<JoystickStateMachine::JoystickState>(joystickSignalToState(analogRead(JOYSTICK_X_IN_1))));
      _joystick1YStateMachine.UpdateState(static_cast<JoystickStateMachine::JoystickState>(joystickSignalToState(analogRead(JOYSTICK_Y_IN_1))));
      //Serial.println(analogRead(JOYSTICK_X_IN_1));
      //Serial.println(analogRead(JOYSTICK_Y_IN_1));
      break;
    default: 
      break;
  }

  // Update State Machines, send chars
  _button0StateMachine.UpdateState(static_cast<ButtonStateMachine::ButtonState>(digitalRead(BUTTON_IN_0)));
  _button1StateMachine.UpdateState(static_cast<ButtonStateMachine::ButtonState>(digitalRead(BUTTON_IN_1)));
  _button2StateMachine.UpdateState(static_cast<ButtonStateMachine::ButtonState>(digitalRead(BUTTON_IN_2)));
  _button3StateMachine.UpdateState(static_cast<ButtonStateMachine::ButtonState>(digitalRead(BUTTON_IN_3)));
  _button4StateMachine.UpdateState(static_cast<ButtonStateMachine::ButtonState>(digitalRead(BUTTON_IN_4)));
  _button5StateMachine.UpdateState(static_cast<ButtonStateMachine::ButtonState>(digitalRead(BUTTON_IN_5)));

  // Printout for debugger
  //Serial.println(digitalRead(BUTTON_IN_0));
  //Serial.println(digitalRead(BUTTON_IN_1));
  //Serial.println(digitalRead(BUTTON_IN_2));
  //Serial.println(digitalRead(BUTTON_IN_3));
  //Serial.println(digitalRead(BUTTON_IN_4));
  //Serial.println(digitalRead(BUTTON_IN_5));  
  //Serial.println("ENDENDENDEND");
  //delay(500);
}

// Main loop
// Purpose:
// Run the clock if needed
// Read input pins
// Map values to states
// Send bluetooth presses
// PERF TODO: Send presses Asynchronously
void loop() {
  // We operate differently based on mode
  switch(_deviceMode)
  {
    case DeviceModes::Gamepad:
      GamepadOperation();
      break;
    case DeviceModes::Keyboard:
      KeyboardOperation();
      break;
    default:
      Serial.println("UNEXPECTED DEVICEMODE");
      delay(10000); 
      break; 
  }
  delay(5);
}
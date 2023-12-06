#include "StateMachines.h"
#include <Arduino.h>

// Binary State Machine for Button
void ButtonStateMachine::UpdateState(ButtonState newState)
{
  switch(newState)
  {
    case OFF:
      switch(_state)
      {
        case ON:
          // BUTTON OFF CODE
          _keyPresser->releaseKey(_key);
          _keyPresser->sendPresses();
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
          _keyPresser->pressKey(_key);
          _keyPresser->sendPresses();
          _state = ON;
          break;
        default:
          break;
      }
      break;
    default:
      Serial.println("UNEXPECTED STATE");
      // BUTTON OFF CODE
      _keyPresser->releaseKey(_key);
      _keyPresser->sendPresses();
      _state = OFF;
      break;  
  }
}

// Trinary State Machine for Joystick axis
void JoystickStateMachine::UpdateState(JoystickState newState)
{
  switch(_state)
  {
    case LEFT:
      switch(newState)
      {
        case OFF:
          _keyPresser->releaseKey(_keyLeft);
          _keyPresser->sendPresses();
          _state = OFF;
          break;
        case RIGHT:
          _keyPresser->releaseKey(_keyLeft);
          _keyPresser->pressKey(_keyRight);
          _keyPresser->sendPresses();
          _state = RIGHT;
          break;
        default:
          break;
      }
      break;
    case RIGHT:
      switch(newState)
      {
        case OFF:
          _keyPresser->releaseKey(_keyRight);
          _keyPresser->sendPresses();
          _state = OFF;
          break;
        case LEFT:
          _keyPresser->releaseKey(_keyRight);
          _keyPresser->pressKey(_keyLeft);
          _keyPresser->sendPresses();
          _state = LEFT;
          break;
        default:
          break;
      }
      break;
    case OFF:
      switch(newState)
      {
        case RIGHT:
          _keyPresser->pressKey(_keyRight);
          _keyPresser->sendPresses();
          _state = RIGHT;
          break;
        case LEFT:
          _keyPresser->pressKey(_keyLeft);
          _keyPresser->sendPresses();
          _state = LEFT;
          break;
        default:
          break;
      }
      break;
    default:
      Serial.println("UNEXPECTED STATE");
      _keyPresser->releaseKey(_keyLeft);
      _keyPresser->releaseKey(_keyRight);
      _keyPresser->sendPresses();
      _state = OFF;
      break;  
  }
}

// Binary State Machine for Gamepad Button
void GamepadButtonStateMachine::UpdateState(ButtonState newState)
{
    switch(newState)
  {
    case OFF:
      switch(_state)
      {
        case ON:
          // BUTTON OFF CODE
          _gamepad->release(_button);
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
            _gamepad->press(_button);
          _state = ON;
          break;
        default:
          break;
      }
      break;
    default:
      Serial.println("UNEXPECTED STATE");
      // BUTTON OFF CODE
      _gamepad->release(_button);
      _state = OFF;
      break;  
  }
}

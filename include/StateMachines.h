#ifndef _STATEMACHINES_H_
#define _STATEMACHINES_H_

#include <BleGamepad.h>
#include "KeyPressStorage.h"

// Binary State Machine for Keyboard Button
class ButtonStateMachine
{
public:
  enum ButtonState
  {
    ON    = 0,
    OFF   = 1,
  };

  ButtonStateMachine(const char key, KeyPressStorage *keyPresser) : 
    _state(ButtonState::OFF), 
    _key(key), 
    _keyPresser(keyPresser) 
    {}

  void UpdateState(ButtonState newState);

private:
  ButtonState _state;
  const char _key;
  KeyPressStorage *_keyPresser;
};

// Trinary State Machine for Keyboard Joystick axis
class JoystickStateMachine
{
public:
  enum JoystickState
  {
    RIGHT  = -1,
    OFF   = 0,
    LEFT = 1
  };

  JoystickStateMachine(const char keyLeft, const char keyRight, KeyPressStorage *keyPresser) : 
  _state(JoystickState::OFF), 
  _keyLeft(keyLeft), 
  _keyRight(keyRight), 
  _keyPresser(keyPresser) 
  {}
  
  void UpdateState(JoystickState newState);

private:
  JoystickState _state;
  const char _keyLeft;
  const char _keyRight;
  KeyPressStorage *_keyPresser;
};

// Binary State Machine for Gamepad Button
class GamepadButtonStateMachine
{
public:
  enum ButtonState
  {
    ON    = 0,
    OFF   = 1,
  };

  GamepadButtonStateMachine(int button, BleGamepad *gamepad) : 
    _state(ButtonState::OFF), 
    _button(button),
    _gamepad(gamepad) 
    {}

  void UpdateState(ButtonState newState);

private:
  ButtonState _state;
  int _button;
  BleGamepad *_gamepad;
};

#endif /* _STATEMACHINES_H_ */
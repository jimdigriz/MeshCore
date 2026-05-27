#pragma once

#include <Arduino.h>
#include <FunctionalInterrupt.h>
#include <helpers/ESP32Board.h>

#define BUTTON_DEBOUNCE_TIME_MS     50 // Debounce time in ms
#define BUTTON_CLICK_TIMEOUT_MS    500 // Max time between clicks for multi-click
#define BUTTON_LONG_PRESS_TIME_MS 3000 // Time to trigger long press (3 seconds)

#ifdef DISPLAY_CLASS
  #ifdef USE_INTERUPT_BUTTON
// https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/GPIO/FunctionalInterrupt/FunctionalInterrupt.ino
class Button {
  uint8_t _pin;

  // TEST POWER USAGE WITH POLLING TO COMPARE!
  enum State {
      IDLE,
      DEBOUNCE,
      WAITING_FOR_MULTI_CLICK,
      CONFIRMED
  };

  volatile bool _pressed;
  volatile State _state;

  void ARDUINO_ISR_ATTR _isr() {
    _pressed = !_pressed;
  }

public:
  Button(uint8_t pin) : _pin(pin) {
    pinMode(_pin, INPUT_PULLUP);
  };

  void begin() {
    _state = IDLE;

    _pressed = digitalRead(_pin);
    attachInterrupt(digitalPinToInterrupt(_pin), std::bind(&Button::_isr, this), CHANGE);
  }
};
  #endif
#endif

class LilyGoTHigrowBoard : public ESP32Board {
#ifdef DISPLAY_CLASS
  #ifdef USE_INTERUPT_BUTTON
  Button _btn_boot;
  Button _btn_user;
  #endif
#endif
public:
#ifdef DISPLAY_CLASS
  #ifdef USE_INTERUPT_BUTTON
  LilyGoTHigrowBoard(const Button& btn_boot, const Button& btn_user) : _btn_boot(btn_boot), _btn_user(btn_user) {}
  #else
  LilyGoTHigrowBoard() {}
  #endif
#else
  LilyGoTHigrowBoard() {}
#endif

  const char* getManufacturerName() const override {
    return "LILYGO T-HiGrow";
  }

  void begin() {
    ESP32Board::begin();

#ifdef DISPLAY_CLASS
  #ifdef USE_INTERUPT_BUTTON
    _btn_boot.begin();
    _btn_user.begin();
  #endif
#endif

    // IO4 must be set high otherwise the ADC (measures the battery)
    // and sensors are not powered and so will not read
    // FIXME use strategy in variants/mesh_pocket/MeshPocket.h though the soil/salt testing also needs this
    pinMode(PIN_BAT_CTL, OUTPUT);
    digitalWrite(PIN_BAT_CTL, HIGH);
    delay(100);
  }
};

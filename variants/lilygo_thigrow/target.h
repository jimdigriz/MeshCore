#pragma once

#define RADIOLIB_STATIC_ONLY 1
#include <RadioLib.h>
#include <helpers/radiolib/RadioLibWrappers.h>
#include <LilyGoTHigrowBoard.h>
#include <helpers/radiolib/CustomSX1276Wrapper.h>
#include <helpers/AutoDiscoverRTCClock.h>
#include <helpers/SensorManager.h>
#include <Adafruit_BME280.h>
#include <BH1750_WE.h>
#include <DHT.h>
#ifdef DISPLAY_CLASS
  #include "helpers/ui/NullDisplayDriver.h"
#endif

class THigrowSensorManager: public SensorManager {
  Adafruit_BME280 _bme280;
  bool _bme280_found;

  DHT _dht;

  BH1750_WE _bh1750;

  float _humidity;
  float _temperature;
  float _pressure;
  float _altitude;
  uint32_t _lux;
  float _soil;
  uint8_t _salt;
public:
  THigrowSensorManager(const DHT& dht) : _dht(dht) {};
  bool begin() override;
  bool querySensors(uint8_t requester_permissions, CayenneLPP& telemetry) override;
  void loop() override;
};

#ifdef DISPLAY_CLASS
  extern NullDisplayDriver display;
#endif
extern LilyGoTHigrowBoard board;
extern WRAPPER_CLASS radio_driver;
extern AutoDiscoverRTCClock rtc_clock;
extern THigrowSensorManager sensors;

bool radio_init();
mesh::LocalIdentity radio_new_identity();

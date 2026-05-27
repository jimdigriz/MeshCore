#include <Arduino.h>
#include <DHT.h>
#include "target.h"

static SPIClass spi;
static RADIO_CLASS radio = new Module(P_LORA_NSS, P_LORA_DIO_0, P_LORA_RESET, P_LORA_DIO_1, spi);
static DHT dht(TELEM_DHT_PIN, DHT11);

THigrowSensorManager sensors(dht);

static ESP32RTCClock fallback_clock;
AutoDiscoverRTCClock rtc_clock(fallback_clock);

#ifdef DISPLAY_CLASS
  NullDisplayDriver display;

  #ifdef USE_INTERUPT_BUTTON
  static Button btn_boot(PIN_BTN_BOOT);
  static Button btn_user(PIN_BTN_USER);
  #endif
#endif

#ifdef DISPLAY_CLASS
  #ifdef USE_INTERUPT_BUTTON
LilyGoTHigrowBoard board(btn_boot, btn_user);
  #else
LilyGoTHigrowBoard board;
  #endif
#else
LilyGoTHigrowBoard board;
#endif

WRAPPER_CLASS radio_driver(radio, board);

// TODO
//  * BUTTONS
//  * RTC?
//  * BLE and WIFI
//  * POWER SAVING

bool THigrowSensorManager::begin() {
  _humidity = 0;
  _temperature = 0;
  _pressure = 0;
  _altitude = 0;
  _lux = 0;
  _soil = 0;
  _salt = 0;

  // taken from EnvironmentSensorManager.cpp but the BME280 code path is untested
  if (_bme280.begin(TELEM_BME280_ADDRESS, &Wire)) {
    MESH_DEBUG_PRINTLN("Found BME280");
    _bme280_found = true;
    _bme280.setSampling(Adafruit_BME280::MODE_FORCED,
                       Adafruit_BME280::SAMPLING_X1,   // temperature
                       Adafruit_BME280::SAMPLING_X1,   // pressure
                       Adafruit_BME280::SAMPLING_X1,   // humidity
                       Adafruit_BME280::FILTER_OFF,
                       Adafruit_BME280::STANDBY_MS_1000);
  } else {
    MESH_DEBUG_PRINTLN("Not found BME280, using DHT11");
    _bme280_found = false;
    _dht.begin();
  }

  _bh1750 = BH1750_WE(TELEM_BH1750_ADDRESS);
  if (!_bh1750.init()) {
    Serial.println("Connection to the BH1750 failed");
    Serial.println("Check wiring and I2C address");
    return false;
  }

  return true;
}

bool THigrowSensorManager::querySensors(uint8_t requester_permissions, CayenneLPP& telemetry) {
  if (!(requester_permissions & TELEM_PERM_ENVIRONMENT))
    return true;

  // TODO detect BME280 and use that instead
  telemetry.addRelativeHumidity(TELEM_CHANNEL_SELF, _humidity);
  telemetry.addTemperature(TELEM_CHANNEL_SELF, _temperature);
  if (_bme280_found) {
    telemetry.addBarometricPressure(TELEM_CHANNEL_SELF, _pressure);
    telemetry.addAltitude(TELEM_CHANNEL_SELF, _altitude);
  }

  telemetry.addLuminosity(TELEM_CHANNEL_SELF, _lux);

  uint8_t channel_ec = TELEM_CHANNEL_SELF + 1;
  telemetry.addRelativeHumidity(channel_ec, _soil);
  telemetry.addPercentage(channel_ec, _salt);

  return true;
}

void THigrowSensorManager::loop() {
  analogReadResolution(12);

  if (_bme280_found) {
    if (_bme280.takeForcedMeasurement()) {
      _temperature = _bme280.readTemperature();
      _humidity = _bme280.readHumidity();
      _pressure = _bme280.readPressure() / 100;
      _altitude = _bme280.readAltitude(1013.25);
    } else {
      MESH_DEBUG_PRINTLN("Failed BME280 measurement");
    }
  } else {
    float humidity = _dht.readHumidity();
    if (isnan(humidity)) {
      MESH_DEBUG_PRINTLN("Failed DHT11 humidity measurement");
    } else {
      _humidity = humidity;
    }

    float temperature = _dht.readTemperature();
    if (isnan(temperature)) {
      MESH_DEBUG_PRINTLN("Failed DHT11 temperature measurement");
    } else {
       _temperature = temperature;
    }
  }

  _bh1750.setMode(OTL);
  delay(20);
  float lux = _bh1750.getLux();
  if (!isnan(lux)) _lux = lux;

  _soil = map(analogRead(SOIL_PIN), 0, 4095, 200, 0) / 2.0f;

  uint8_t samples = 120;
  uint32_t humi = 0;
  uint16_t array[120];
  for (int i = 0; i < samples; i++) {
    array[i] = analogRead(SALT_PIN);
    delay(2);
  }
  std::sort(array, array + samples);
  for (int i = 1; i < samples - 1; i++) {
    humi += array[i];
  }
  _salt = map(humi / (samples - 2), 0, 4095, 0, 100);
}

bool radio_init() {
  fallback_clock.begin();
  rtc_clock.begin(Wire);

  spi.begin(P_LORA_SCLK, P_LORA_MISO, P_LORA_MOSI);
  return radio.std_init(&spi);
}

mesh::LocalIdentity radio_new_identity() {
  RadioNoiseListener rng(radio);
  return mesh::LocalIdentity(&rng);  // create new random identity
}

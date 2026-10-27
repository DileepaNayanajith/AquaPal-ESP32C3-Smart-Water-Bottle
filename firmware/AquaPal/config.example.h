#pragma once

// Copy to config.h before compiling. config.h is intentionally gitignored.
// These are example pins for a new build, NOT a record of the 2025 wiring.
#define AQUAPAL_AP_SSID "AquaPal"
#define AQUAPAL_AP_PASSWORD "replace-with-your-password"  // 8+ characters

#define PIN_I2C_SDA 6
#define PIN_I2C_SCL 7
#define PIN_HCSR04_TRIG 5
#define PIN_HCSR04_ECHO 4
#define PIN_DHT11 3
#define PIN_TTP223 1
#define PIN_BUZZER 0

#define OLED_I2C_ADDRESS 0x3C

// MEASURE these on your actual installed sensor before relying on volume data.
// Distance is between sensor face and surface of water, in millimetres.
#define DISTANCE_EMPTY_MM 220.0f
#define DISTANCE_FULL_MM 40.0f
#define BOTTLE_CAPACITY_ML 2000

/*
  AquaPal - ESP32-C3 SuperMini demonstration firmware (reconstructed in 2026)

  This is NOT a recovered copy of the firmware used in the September 2025 video.
  It is a fresh implementation of the documented sensor/dashboard idea.
  Check hardware pin mapping and water-level calibration before powering it.
*/

#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <DHT.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"  // local copy of config.example.h, not checked into Git
#include "HydrationTracker.h"
#include "Dashboard.h"

static_assert(DISTANCE_EMPTY_MM > DISTANCE_FULL_MM,
              "Empty distance must exceed full distance");

static constexpr uint32_t SENSOR_PERIOD_MS = 3000;
static constexpr uint32_t OLED_PERIOD_MS = 350;
static constexpr uint32_t SAVE_PERIOD_MS = 60000;
static constexpr uint32_t REMINDER_REPEAT_MS = 5UL * 60000UL;

Adafruit_SSD1306 oled(128, 64, &Wire, -1);
DHT dht(PIN_DHT11, DHT11);
RTC_DS3231 rtc;
WebServer web(80);
Preferences preferences;
HydrationTracker hydration(BOTTLE_CAPACITY_ML);

bool hasOled = false;
bool hasRtc = false;
bool hasUnsavedChanges = false;
bool previousTouch = false;
uint8_t oledPage = 0;
uint16_t reminderMinutes = 30;
uint32_t todayKey = 0;

float ambientTempC = NAN;
float ambientHumidity = NAN;
float lastDistanceMm = NAN;
unsigned long lastValidSensorMs = 0;
unsigned long lastSensorMs = 0;
unsigned long lastOledMs = 0;
unsigned long lastSaveMs = 0;
unsigned long lastDailyCheckMs = 0;
unsigned long lastTouchMs = 0;
unsigned long lastDrinkOrDismissMs = 0;
unsigned long lastReminderMs = 0;
unsigned long buzzerOffMs = 0;

static uint32_t dateKey(const DateTime& timestamp) {
  return static_cast<uint32_t>(timestamp.year()) * 10000UL
      + static_cast<uint32_t>(timestamp.month()) * 100UL + timestamp.day();
}

static float distanceMmToVolume(float distanceMm) {
  // Linear volume-depth approximation. Calibrate/replace for non-uniform bottle shapes.
  float fraction = (DISTANCE_EMPTY_MM - distanceMm)
                 / (DISTANCE_EMPTY_MM - DISTANCE_FULL_MM);
  if (fraction < 0) fraction = 0;
  if (fraction > 1) fraction = 1;
  return fraction * BOTTLE_CAPACITY_ML;
}

static float measureDistanceMm() {
  float readings[5];
  int count = 0;
  for (int i = 0; i < 5; ++i) {
    digitalWrite(PIN_HCSR04_TRIG, LOW);
    delayMicroseconds(3);
    digitalWrite(PIN_HCSR04_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_HCSR04_TRIG, LOW);

    const unsigned long echoUs = pulseIn(PIN_HCSR04_ECHO, HIGH, 30000UL);
    if (echoUs > 0) {
      const float mm = echoUs * 0.343f * 0.5f;
      if (mm >= 20.0f && mm <= 4000.0f) readings[count++] = mm;
    }
    delay(20);
  }
  if (count < 3) return NAN;

  // Five-sample median rejects isolated echoes.
  for (int i = 1; i < count; ++i) {
    const float n = readings[i];
    int j = i - 1;
    while (j >= 0 && readings[j] > n) {
      readings[j + 1] = readings[j];
      --j;
    }
    readings[j + 1] = n;
  }
  return readings[count / 2];
}

static void saveStats(bool force = false) {
  if (!force && !hasUnsavedChanges) return;
  preferences.putUInt("consumed", hydration.consumedMl());
  if (todayKey != 0) preferences.putUInt("day", todayKey);
  hasUnsavedChanges = false;
  lastSaveMs = millis();
}

static void checkNewDay() {
  if (!hasRtc) return;
  const uint32_t current = dateKey(rtc.now());
  if (current != todayKey) {
    todayKey = current;
    hydration.resetDaily();
    saveStats(true);
    Serial.println("New calendar day: hydration counter reset");
  }
}

static bool reminderDue() {
  const uint32_t window = static_cast<uint32_t>(reminderMinutes) * 60000UL;
  return millis() - lastDrinkOrDismissMs >= window;
}

static void buzzBriefly() {
  digitalWrite(PIN_BUZZER, HIGH);
  buzzerOffMs = millis() + 180;
}

static void updateBuzzerAndTouch() {
  const unsigned long now = millis();
  if (buzzerOffMs != 0 && static_cast<int32_t>(now - buzzerOffMs) >= 0) {
    digitalWrite(PIN_BUZZER, LOW);
    buzzerOffMs = 0;
  }

  const bool touched = digitalRead(PIN_TTP223) == HIGH;
  if (touched && !previousTouch && now - lastTouchMs > 250) {
    oledPage = (oledPage + 1) % 3;
    lastTouchMs = now;
    // A touch acknowledges the reminder, but does not count as water consumed.
    lastDrinkOrDismissMs = now;
    lastReminderMs = now;
    digitalWrite(PIN_BUZZER, LOW);
    buzzerOffMs = 0;
  }
  previousTouch = touched;

  if (reminderDue() && (lastReminderMs == 0 || now - lastReminderMs >= REMINDER_REPEAT_MS)) {
    buzzBriefly();
    lastReminderMs = now;
  }
}

static void readSensors() {
  const float distance = measureDistanceMm();
  if (isfinite(distance)) {
    lastDistanceMm = distance;
    lastValidSensorMs = millis();
    const HydrationTracker::Event event = hydration.observe(distanceMmToVolume(distance));
    if (event == HydrationTracker::DRINK_ESTIMATED) {
      lastDrinkOrDismissMs = millis();
      lastReminderMs = 0;
      hasUnsavedChanges = true;
      Serial.printf("Estimated daily intake: %lu mL\n", (unsigned long)hydration.consumedMl());
    } else if (event == HydrationTracker::REFILL_ESTIMATED) {
      Serial.println("Water level increased; treated as a possible refill");
    }
  }

  const float t = dht.readTemperature();
  const float h = dht.readHumidity();
  if (isfinite(t)) ambientTempC = t;
  if (isfinite(h)) ambientHumidity = h;
}

static void drawOled() {
  if (!hasOled) return;
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.println(F("AquaPal / ESP32-C3"));
  oled.drawLine(0, 11, 127, 11, SSD1306_WHITE);

  if (oledPage == 0) {
    oled.setCursor(0, 17);
    oled.println(F("Water remaining"));
    oled.setTextSize(2);
    oled.setCursor(0, 30);
    if (hydration.initialized()) oled.printf("%u mL", hydration.levelMl());
    else oled.print(F("Waiting..."));
    oled.setTextSize(1);
    oled.setCursor(0, 56);
    oled.print(F("Touch to change page"));
  } else if (oledPage == 1) {
    oled.setCursor(0, 19);
    oled.printf("Consumed: %lu mL\n", (unsigned long)hydration.consumedMl());
    oled.printf("Goal:     %u mL\n", hydration.goalMl());
    oled.printf("Reminder: %u min\n", reminderMinutes);
    oled.println(reminderDue() ? F("Drink water!") : F("On schedule"));
  } else {
    oled.setCursor(0, 20);
    if (isfinite(ambientTempC)) oled.printf("Temp: %.1f C\n", ambientTempC);
    else oled.println(F("Temp: unavailable"));
    if (isfinite(ambientHumidity)) oled.printf("Humidity: %.0f %%\n", ambientHumidity);
    else oled.println(F("Humidity: unavailable"));
    if (hasRtc) {
      DateTime n = rtc.now();
      oled.printf("Time: %02u:%02u:%02u", n.hour(), n.minute(), n.second());
    } else oled.println(F("RTC not detected"));
  }
  oled.display();
}

static void routeStatus() {
  const bool sensorReady = hydration.initialized() && (millis() - lastValidSensorMs < 15000UL);
  const uint32_t windowMs = static_cast<uint32_t>(reminderMinutes) * 60000UL;
  const uint32_t elapsedMs = millis() - lastDrinkOrDismissMs;
  const uint32_t minutesRemaining = elapsedMs >= windowMs ? 0 : (windowMs - elapsedMs + 59999UL) / 60000UL;
  String timeString = "";
  if (hasRtc) {
    DateTime n = rtc.now();
    char timeBuf[12];
    snprintf(timeBuf, sizeof(timeBuf), "%02u:%02u:%02u", n.hour(), n.minute(), n.second());
    timeString = timeBuf;
  }
  String json = "{";
  json += "\"sensorReady\":" + String(sensorReady ? "true" : "false");
  json += ",\"waterMl\":" + String(hydration.levelMl());
  json += ",\"consumedMl\":" + String(hydration.consumedMl());
  json += ",\"goalMl\":" + String(hydration.goalMl());
  json += ",\"reminderIntervalMin\":" + String(reminderMinutes);
  json += ",\"nextReminderMin\":" + String(minutesRemaining);
  json += ",\"reminderDue\":" + String(reminderDue() ? "true" : "false");
  json += ",\"temperatureC\":" + (isfinite(ambientTempC) ? String(ambientTempC, 1) : String("null"));
  json += ",\"humidity\":" + (isfinite(ambientHumidity) ? String(ambientHumidity, 1) : String("null"));
  json += ",\"time\":\"" + timeString + "\"";
  json += "}";
  web.sendHeader("Cache-Control", "no-store");
  web.send(200, "application/json", json);
}

static void routeSettings() {
  if (!web.hasArg("goalMl") || !web.hasArg("reminderMinutes")) {
    web.send(400, "text/plain", "Missing goalMl or reminderMinutes");
    return;
  }
  const long goal = web.arg("goalMl").toInt();
  const long minutes = web.arg("reminderMinutes").toInt();
  if (goal < 250 || goal > 5000 || minutes < 5 || minutes > 180) {
    web.send(400, "text/plain", "Goal or reminder interval outside supported range");
    return;
  }
  hydration.setGoal(static_cast<uint16_t>(goal));
  reminderMinutes = static_cast<uint16_t>(minutes);
  preferences.putUShort("goal", hydration.goalMl());
  preferences.putUShort("reminder", reminderMinutes);
  lastDrinkOrDismissMs = millis();
  lastReminderMs = millis();
  web.send(200, "text/plain", "Saved");
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_HCSR04_TRIG, OUTPUT);
  pinMode(PIN_HCSR04_ECHO, INPUT);  // MUST be reduced to 3.3 V if sensor echo is 5 V
  pinMode(PIN_TTP223, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_HCSR04_TRIG, LOW);
  digitalWrite(PIN_BUZZER, LOW);

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  dht.begin();
  hasOled = oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS);
  hasRtc = rtc.begin();
  if (hasRtc && rtc.lostPower()) {
    // Initial setup fallback only. Set RTC precisely for accurate daily rollover.
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.println("RTC lost power: set to sketch compilation timestamp");
  }

  preferences.begin("aquapal", false);
  hydration.setGoal(preferences.getUShort("goal", 2000));
  reminderMinutes = preferences.getUShort("reminder", 30);
  if (hydration.goalMl() < 250 || hydration.goalMl() > 5000) hydration.setGoal(2000);
  if (reminderMinutes < 5 || reminderMinutes > 180) reminderMinutes = 30;

  if (hasRtc) {
    todayKey = dateKey(rtc.now());
    if (preferences.getUInt("day", 0) == todayKey)
      hydration.restoreConsumed(preferences.getUInt("consumed", 0));
    else saveStats(true);
  } else {
    // Without a reliable RTC, do not restore a previous day's total.
    hydration.resetDaily();
    Serial.println("RTC missing: daily statistics are not persisted between boots");
  }

  // SoftAP works offline. Change the password in ignored config.h before upload.
  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(AQUAPAL_AP_SSID, AQUAPAL_AP_PASSWORD)) {
    Serial.println("Wi-Fi AP failed to start");
  } else {
    Serial.printf("Connect to %s, then visit http://%s\n",
                  AQUAPAL_AP_SSID, WiFi.softAPIP().toString().c_str());
  }
  web.on("/", HTTP_GET, []() { web.send_P(200, "text/html; charset=utf-8", DASHBOARD_HTML); });
  web.on("/api/status", HTTP_GET, routeStatus);
  web.on("/api/settings", HTTP_POST, routeSettings);
  web.onNotFound([]() { web.send(404, "text/plain", "Not found"); });
  web.begin();

  lastDrinkOrDismissMs = millis();
  lastReminderMs = millis();
  Serial.println("AquaPal demo firmware ready");
}

void loop() {
  web.handleClient();
  updateBuzzerAndTouch();
  const unsigned long now = millis();

  if (now - lastSensorMs >= SENSOR_PERIOD_MS) {
    lastSensorMs = now;
    readSensors();
  }
  if (now - lastOledMs >= OLED_PERIOD_MS) {
    lastOledMs = now;
    drawOled();
  }
  if (now - lastDailyCheckMs >= 5000) {
    lastDailyCheckMs = now;
    checkNewDay();
  }
  if (hasUnsavedChanges && now - lastSaveMs >= SAVE_PERIOD_MS) saveStats();
}

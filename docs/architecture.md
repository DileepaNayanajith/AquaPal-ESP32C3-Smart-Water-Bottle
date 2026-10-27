# Reference implementation architecture

AquaPal's later reconstructed firmware keeps everything on the ESP32-C3 and does not need cloud services.

```text
  HC-SR04                 DHT11             DS3231
    |                       |                 |
    v                       v                 v
 ultrasonics         ambient conditions     RTC clock
    |                       |                 |
 median filter               |                 |
    v                        v                 v
 volume estimate ------ ESP32-C3 controller --------- TTP223 touch
    |                        |
    v                        +------> SSD1306 OLED
 stability & threshold logic +------> buzzer reminders
    |                        |
 estimated intake            +------> Wi-Fi SoftAP
    |                                      |
 local daily count                    HTTP web server
    |                                      |
 Preferences (NVS) <------------------ dashboard / settings
```

**Firmware files:** `AquaPal.ino` handles input/output, persistence and scheduling. `HydrationTracker.h` holds an independently testable water-level state machine. `Dashboard.h` contains the offline browser interface. `config.example.h` collects the wiring and calibration parameters.

**Persistence:** daily consumed mL is saved periodically to NVS and reloaded on boot only when the RTC reports the same day. The example sets an RTC that has lost power to the sketch compilation time; check it before relying on daily rollover. Saving is throttled to avoid writing flash every sensor cycle.

**Limitations:** a stationary, upright bottle is assumed for distance-based tracking. The source also has a simple repeated-measurement stability check, which is not a substitute for an IMU.

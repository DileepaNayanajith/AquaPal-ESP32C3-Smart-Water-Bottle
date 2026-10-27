# Testing and limitations

## Verified from the material available

- The supplied 20-second demo video shows an electronics prototype, cylindrical CAD enclosure views, the assembled bottle, and a browser dashboard branded AquaPal.
- The desktop C++ tracking-logic test in `tests/test_tracker.cpp` has passed locally.

## Not yet verified

- Arduino IDE or PlatformIO compilation of the full sketch with the ESP32 library versions on a real machine.
- Exact GPIO mapping and working logic levels of the September 2025 build.
- The original firmware source and original CAD/STL files.
- Real water-volume calibration and accuracy against measured millilitres.
- Reminders, RTC accuracy after power loss, and hardware behaviour under sustained use.
- Power consumption and battery runtime (no battery runtime is claimed).

## How to validate it

1. Verify wiring and safe signal voltages before applying power.
2. Confirm SSD1306 and DS3231 I²C addresses with an I²C scanner.
3. Check DHT11 readings in the Serial Monitor.
4. With the bottle upright, measure distance at empty and full levels. Enter the real values in `config.h`.
5. Add known measured water volumes in 200 mL steps. Record actual versus predicted volume and calculate the error.
6. Test repeated readings at a steady level, and then try deliberate bottle movement and tilting. Note any false consumption events.
7. Verify the touch input and buzzer wiring. Test a short reminder interval before switching to the desired value.
8. Check a refill event, a day's date rollover, power cycling, and preservation of the daily counter.
9. Open the access-point dashboard from a phone and confirm the displayed metrics and form settings.

### Why water consumed is an estimate

HC-SR04 measures distance to the water surface, not water volume directly. The included example maps depth linearly to volume, which is only a rough approximation for a uniform cross-section container. An irregular bottle needs a calibration table or geometry-aware conversion. A level drop could be caused by actual drinking, movement, foam, or tilt; this design does not include a motion sensor to distinguish those cases. It should not be used for medical decisions.

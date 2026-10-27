# Hardware and example wiring

This file documents a **suggested wiring arrangement for the new reference sketch**. The pin numbers below are **not confirmed** as the exact wiring used in the 16 September 2025 prototype. Do not connect parts based on a guessed original pinout.

| Module signal | ESP32-C3 SuperMini example pin |
| --- | --- |
| SSD1306 SDA + DS3231 SDA | GPIO 6 |
| SSD1306 SCL + DS3231 SCL | GPIO 7 |
| HC-SR04 TRIG | GPIO 5 |
| HC-SR04 ECHO (after 5 V to 3.3 V divider / shifter) | GPIO 4 |
| DHT11 DATA | GPIO 3 |
| TTP223 OUT | GPIO 1 |
| Buzzer driver input | GPIO 0 |

### Electrical notes

- **Important:** The usual HC-SR04 module is powered from 5 V and can output a 5 V Echo signal. The ESP32-C3 pins are 3.3 V logic and are not 5 V tolerant. Use a level shifter or resistor divider on Echo; for example, 10 kΩ from Echo to GPIO and 20 kΩ from GPIO to GND (approximately 3.3 V from a 5 V input).
- Some 5 V ultrasonic modules do not reliably recognize a 3.3 V trigger. Verify yours or use a 3.3 V-compatible sensor or suitable logic-level conversion.
- Check OLED and RTC module operating voltage and pull-up resistors before putting them on the 3.3 V I²C bus. Use a compatible DS3231 breakout.
- Use a proper transistor/driver for a buzzer that draws more current than a microcontroller GPIO can safely supply.
- Power supply, grounding, connectors, and protection should be checked for the actual assembly. The video is not enough to reconstruct those connections safely.
- Keep moisture away from the ESP32 and exposed conductors; this is a demonstrator, not a waterproof consumer product.

### Calibration

Open `firmware/AquaPal/config.h` after copying the example. Set `BOTTLE_CAPACITY_ML`, `DISTANCE_EMPTY_MM`, and `DISTANCE_FULL_MM` using real measurements. These values in the example are placeholders and may be wrong for the original bottle.

# Firmware

`AquaPal/AquaPal.ino` is a **new 2026 reference implementation**, not a copy of the original 2025 sketch. It needs the hardware pin assignment, correct voltage protection, and water-level calibration for your own physical build.

Follow the setup instructions in the root [README](../README.md). `config.h` is intentionally excluded from version control because it may contain a Wi-Fi access point password.

The only automated test bundled here is a desktop C++ test of the hardware-independent `HydrationTracker.h` algorithm. A successful desktop test does not guarantee the firmware compiles or works on a physical ESP32-C3.

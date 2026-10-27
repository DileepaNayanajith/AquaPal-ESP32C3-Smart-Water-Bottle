# Project history and provenance

**2025-09-16 — AquaPal project built.** The project owner dates the smart water bottle prototype to 16 September 2025. It used an ESP32-C3 SuperMini with ultrasonic sensing, an OLED, a DHT11, clock, touch input, buzzer, and a bottle enclosure.

**Original demo material.** `media/AquaPal-demo-2025.mp4` is an unmodified copy of the video uploaded by the project owner. The `.jpg` files in `media/` are video frames extracted from that same footage. They are not independently captured photographs, generated pictures, or an edited reenactment.

**2026 — GitHub documentation and code reconstruction.** The repository README, technical notes, tests, and the `firmware/AquaPal/` implementation were prepared later to document the historical build and supply an example. The original 2025 sketch and CAD source files were not provided. No claim is made that the reconstructed code is byte-for-byte identical or even feature-identical to the 2025 firmware.

If the original sketch or CAD/STL files are found later, they can be added as a separate historical source without replacing this provenance note.

## Publication checks — 27 October 2026

- The packaged MP4 matches the separately supplied original byte for byte (SHA-256: `fbea9f858a84f8fe942ad7642ce821cd0cae754a70667a391bd199c515c377e7`).
- All four JPEGs were visually checked against scenes in the source video; no stock or generated project images were found.
- Source and documentation were reviewed for credentials. Only a placeholder access-point password is included; local `config.h` is ignored.
- The portable C++ tracking test passed locally with warnings enabled. Full Arduino compilation and physical hardware testing were not performed during publication.
- The historical build date is supplied by the owner; publication commits use the current date.

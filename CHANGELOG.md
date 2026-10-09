# Changelog

All notable changes to the CYD Project Scaffold. Dates are DD-MM-YYYY.

---

## [1.2.0] – unreleased

---

## [1.1.0] 09-10-2026

### Added

- Browser installer through [cyd-web-installer](https://github.com/anthonyjclarke/cyd-web-installer):
  `.github/workflows/firmware.yml` builds both boards on every push and, on a
  `v*` tag on `main`, publishes the release and the GitHub Pages installer.
- Installer labels for `cyd28` (2.8″ ESP32-2432S028R) and `cyd40` (4.0″ ESP32-32E).
- `tools/merge_bin.py` post-build script (`flash_parts.json`, `firmware-merged.bin`).
- Improv-Serial WiFi setup, always on (`lib/ImprovWiFi`, `src/network/improv_setup.*`);
  the WiFiManager portal now runs non-blocking so Improv answers during it.

### Fixed

- Installer Connect offered **Install** instead of **Update** on a provisioned
  board: Improv replies now start on a new line, because ESP Web Tools drops a
  packet that follows serial noise. `lib/ImprovWiFi` is the cyd-web-installer
  1.0.1 copy-in, which carries this fix.
- `PROJECT_NAME` in `include/config.h`, and boot log lines for the version and
  the running app partition.
- README: Install section and how a new project copied from the scaffold
  becomes installer-ready.

### Changed

- Platform pinned to `espressif32@6.12.0` (Arduino core 2.0.17); the backlight
  is back on the core 2.x LEDC channel API.
- Partition table: `default.csv` → standard dual-OTA `partitions_custom.csv`.
  **The first install of 1.1.0 must erase the board**; NVS settings stay at the
  same offset, but the app slots move.
- CLAUDE.md condensed.

---

## [1.0.0] 21-04-2026

### Added

- Scaffold for `cyd28` and `cyd40` with display, touch, WiFi, ArduinoOTA, RGB
  LED and NVS services, and the hardware validation app.

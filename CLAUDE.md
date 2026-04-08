# Project: CYD Display Test — Platform Scaffold

## Overview
A reusable ESP32 project scaffold targeting the CYD (Cheap Yellow Display) family. It supports two boards (`cyd28` and `cyd40`) selected by a single build flag. The default app is a hardware validation checklist that exercises display, touch, WiFi, OTA, and LED before project-specific work begins. All services live under the `cyd::` namespace; application code goes in `src/app/app.cpp`.

## Hardware

### cyd28 — ESP32-2432S028R (2.8 inch)
- MCU: ESP32-D0WDQ6, dual-core 240 MHz, 4 MB flash, no PSRAM
- Display: ILI9341, 240×320 native, landscape = 320×240
- Touch: XPT2046 on a dedicated SPI bus (not shared with TFT)
- RGB LED: GPIO 4/16/17, active LOW
- LDR: GPIO 34 (ADC input only)
- Power: USB (CP2102)

### cyd40 — LCDWiki ESP32-32E (4.0 inch)
- MCU: ESP32, 4 MB flash
- Display: ST7796S, 320×480 native, landscape = 480×320
- Touch: XPT2046 on the shared TFT SPI bus (via TFT_eSPI raw touch)
- No RGB LED, no LDR
- Power: USB

## Build Environment
- Framework: Arduino
- Platform: espressif32
- Board: esp32dev (both envs)
- Default env: `cyd28`

Key libraries:
- `bodmer/TFT_eSPI @ ^2.5.43`
- `tzapu/WiFiManager @ ^2.0.17`
- `PaulStoffregen/XPT2046_Touchscreen` (GitHub, cyd28 only)
- Built-in: `WiFi`, `ArduinoOTA`, `Preferences`, `SPI`

TFT_eSPI is configured entirely via `-include include/config.h` — never edit `User_Setup.h` directly.

`SPI_FREQUENCY` is 27 MHz in this project (conservative; global rules suggest 55 MHz — increase only after confirming stability). `SPI_TOUCH_FREQUENCY` is 2.5 MHz max and must not be raised.

Built-in fonts are loaded (`LOAD_GLCD`, `LOAD_FONT2`–8) — appropriate for the scaffold/diagnostic app. When forking for a real project, switch to VLW fonts and remove the `LOAD_*` defines.

## Project Structure
```
CYD_Display_Test/
├── platformio.ini              # two envs: cyd28, cyd40 — board selected by build flag
├── include/
│   ├── config.h                # board-select, TFT_eSPI setup, pins, touch calibration, defaults
│   ├── board_profile.h         # runtime BoardProfile struct (cyd:: namespace)
│   ├── debug.h                 # leveled serial macros: DBG_ERROR/WARN/INFO/VERBOSE
│   ├── backlight_service.h     # PWM backlight control
│   ├── display_service.h       # TFT_eSPI wrapper, boot/portal screens, tft() accessor
│   ├── touch_service.h         # board-abstracted touch state and polling
│   ├── wifi_service.h          # WiFiManager connection flow + status
│   ├── ota_service.h           # ArduinoOTA wrapper
│   ├── rgb_led_service.h       # onboard RGB LED (cyd28 only)
│   ├── storage_service.h       # Preferences-backed boot count, device name, debug level
│   ├── secrets.h               # gitignored — WiFi credentials (APP_WIFI_DEFAULT_SSID/PASS)
│   └── app/app.h               # project app entry points: begin() / update(nowMs)
└── src/
    ├── main.cpp                # service init order + main loop
    ├── backlight_service.cpp
    ├── display_service.cpp
    ├── touch_service.cpp
    ├── wifi_service.cpp
    ├── ota_service.cpp
    ├── rgb_led_service.cpp
    ├── storage_service.cpp
    └── app/app.cpp             # default validation checklist → motion demo app
```

## Pin Mapping

### cyd28

| Function    | GPIO | Notes                              |
|:------------|:-----|:-----------------------------------|
| TFT MOSI    | 13   |                                    |
| TFT SCLK    | 14   |                                    |
| TFT CS      | 15   |                                    |
| TFT DC      | 2    |                                    |
| TFT RST     | –1   | not wired                          |
| TFT BL      | 21   | PWM backlight                      |
| Touch CS    | 33   | dedicated SPI bus                  |
| Touch CLK   | 25   |                                    |
| Touch MOSI  | 32   |                                    |
| Touch MISO  | 39   | input-only GPIO                    |
| Touch IRQ   | –1   | not used                           |
| LED R       | 4    | active LOW                         |
| LED G       | 16   | active LOW                         |
| LED B       | 17   | active LOW                         |
| LDR         | 34   | ADC input-only                     |

### cyd40

| Function    | GPIO | Notes                              |
|:------------|:-----|:-----------------------------------|
| TFT MOSI    | 13   |                                    |
| TFT SCLK    | 14   |                                    |
| TFT CS      | 15   |                                    |
| TFT DC      | 2    |                                    |
| TFT RST     | –1   | not wired                          |
| TFT BL      | 27   |                                    |
| Touch CS    | 33   | shared TFT SPI bus                 |
| Touch IRQ   | 36   | input-only GPIO                    |

## Configuration
- All board config in `include/config.h` — pins, calibration, timeouts, debug level default
- Board selected at build time: `-D CYD_BOARD_28` or `-D CYD_BOARD_40`
- Credentials in `include/secrets.h` (gitignored): `APP_WIFI_DEFAULT_SSID` / `APP_WIFI_DEFAULT_PASSWORD`
- Persisted settings (NVS): boot count, device name, debug level — namespace `cyd-core`
- Key tuneable constants in `config.h`:
  - `APP_DEFAULT_DEBUG_LEVEL` (default 3)
  - `APP_DEFAULT_BRIGHTNESS` (default 255)
  - `APP_WIFI_PORTAL_TIMEOUT_SEC` (default 180)
  - `APP_WIFI_CONNECT_TIMEOUT_SEC` (default 20)
  - `APP_OTA_PORT` (default 3232)
  - `SPI_FREQUENCY` (27 MHz), `SPI_TOUCH_FREQUENCY` (2.5 MHz)

## Current State
Stable scaffold. Both `cyd28` and `cyd40` have been flashed and validated on real hardware. The default app shows a 5-bar hardware checklist on boot, then auto-transitions to a bouncing-ball motion demo once display, touch, and bounds checks all pass. No project-specific feature code exists yet — the scaffold is ready to fork for a real project.

## Architecture Notes
- **Service pattern**: every subsystem exposes `begin()` / `update(nowMs)` / `status()` — called from `main.cpp` in a fixed order. Note: `backlight` and `display` do not have `update()`.
- **Board selection**: compile-time macros in `config.h` drive both TFT_eSPI setup and touch backend selection via `#if TOUCH_DEDICATED_SPI`
- **Runtime board metadata**: `cyd::boardProfile()` returns a const `BoardProfile` struct — use this for adaptive layout rather than `#ifdef` in app code. `hasTouch` is hardcoded `true` for both boards.
- **Touch pipeline**: raw ADC → axis transform (swap/invert per board) → calibrated screen coordinates via `mapConstrained()`
- **WiFi**: tries `secrets.h` credentials first (blocking, up to `APP_WIFI_CONNECT_TIMEOUT_SEC`), then falls back to WiFiManager captive portal; portal name is `<deviceName>-setup`
- **OTA**: enabled only if WiFi is connected at the time `cyd::ota::begin()` is called. If WiFi connects later (e.g., after portal), OTA will not self-enable — there is no reconnect path.
- **RGB LED**: `rgb::update()` cycles R→G→B on a 1200 ms phase timer. This is scaffold demo behaviour — apps will need to replace or extend this.
- **Backlight**: uses LEDC channel 0, 5 kHz, 8-bit. Avoid using LEDC channel 0 in app code to prevent conflicts.
- **Config injection**: TFT_eSPI config is injected via `-include include/config.h` in `build_flags`, so no `User_Setup.h` edits are needed
- **BGR colour order**: `TFT_RGB_ORDER TFT_BGR` is defined in `config.h`. CYD panels are wired BGR; removing this define swaps red and blue across all rendering.
- **`debugLevel` global**: declared in `main.cpp`, `extern`-referenced in `debug.h`. Loaded from NVS on boot, adjustable at runtime via `cyd::storage::setDebugLevel()`.
- **App layer**: replace `src/app/app.cpp` with project code; keep all scaffold services intact
- **`loop()` cadence**: a `delay(10)` at the bottom of `loop()` caps the loop to ~100 Hz. Remove or reduce it for higher-frequency updates in real projects.

## Known Issues / Quirks
- `display_service.cpp` implements `drawValidationDemo()` / `updateValidationDemo()` and they are declared in the header, but `app.cpp` draws its own bars directly via `tft()` and never calls them. These are **dead code** — remove from header and implementation when forking.
- `delay(1)` / `delay(2)` in `touch_service.cpp` (cyd40 shared-SPI path) is intentional debounce — not a loop bloat issue.
- `wifi_service.cpp` `trySecretsWifi()` uses a blocking `while` + `delay(200)` loop during connection attempt — WiFi connection at boot is inherently blocking in this scaffold.
- OTA is only armed if WiFi is connected at boot. No runtime re-enable path exists.
- No `partitions_custom.csv` in this scaffold; default partition table is used. Add one before building a project with OTA + large assets.
- `CHANGELOG.md` is not present — create one before making functional changes.

## TODO
- [ ] Add `partitions_custom.csv` when forking for a real project
- [ ] Add `CHANGELOG.md`
- [x] Update `.gitignore` to include `*.bin`, `*.elf`
- [ ] Replace `src/app/app.cpp` with project-specific feature code
- [ ] Consider adding touch calibration UI stored in NVS (currently hardcoded values per board)

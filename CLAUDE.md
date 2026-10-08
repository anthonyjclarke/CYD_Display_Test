# Project: CYD Display Test — Platform Scaffold

CYD (Cheap Yellow Display) project scaffold targeting ESP32-based boards (`cyd28` 2.8″ and `cyd40` 4.0″) selected by compile-time flag. Provides reusable service layer (display, touch, WiFi, OTA, LED, storage) under `cyd::` namespace. Default app is hardware validation checklist. v1.0 stable on both boards.

## Hardware

**cyd28** — ESP32-2432S028R (2.8″)
- Display: ILI9341, 240×320 native (landscape 320×240)
- Touch: XPT2046 on dedicated SPI bus (pins 33/25/32/39)
- RGB LED: GPIO 4/16/17, active LOW
- LDR: GPIO 34 (ADC input-only)
- Backlight: GPIO 21 PWM

**cyd40** — LCDWiki ESP32-32E (4.0″)
- Display: ST7796S, 320×480 native (landscape 480×320)
- Touch: XPT2046 on shared TFT SPI bus (pins 33/36 CS/IRQ)
- No RGB LED, no LDR
- Backlight: GPIO 27 PWM

Board selected at build time: `-D CYD_BOARD_28` or `-D CYD_BOARD_40`. Both boards use `esp32dev` in platformio.ini.

## Pin Mapping

**Non-standard pins only** (shared TFT SPI: GPIO 13/14/15/2 standard across CYD family):

| cyd28   | GPIO | cyd40   | GPIO | Notes |
|:--------|:-----|:--------|:-----|:------|
| TFT BL  | 21   | TFT BL  | 27   | PWM |
| Touch CS | 33   | Touch CS | 33   | dedicated (28) / shared (40) |
| Touch CLK | 25   | —       | —    | cyd28 only |
| Touch MOSI | 32 | —       | —    | cyd28 only |
| Touch MISO | 39 | —       | —    | input-only, cyd28 only |
| LED R   | 4    | —       | —    | active LOW, cyd28 only |
| LED G   | 16   | —       | —    | active LOW, cyd28 only |
| LED B   | 17   | —       | —    | active LOW, cyd28 only |
| LDR     | 34   | —       | —    | ADC, cyd28 only |
| —       | —    | Touch IRQ | 36 | input-only, cyd40 only |

## Libraries
- `bodmer/TFT_eSPI @ ^2.5.43` — configured entirely via `-include include/config.h`; never edit `User_Setup.h`
- `tzapu/WiFiManager @ ^2.0.17`
- `PaulStoffregen/XPT2046_Touchscreen` (GitHub, cyd28 only)

## Configuration & Secrets

**include/config.h** — board selection, TFT setup, pins, calibration, timeouts:
- `CYD_BOARD_28` or `CYD_BOARD_40` (compile flag)
- `APP_DEFAULT_DEBUG_LEVEL` (default 3)
- `APP_DEFAULT_BRIGHTNESS` (default 255)
- `SPI_FREQUENCY` 27 MHz (conservative; global rules allow 55 MHz — test stability before increasing)
- `SPI_TOUCH_FREQUENCY` 2.5 MHz max (hardcoded by XPT2046 lib, do not raise)
- `TFT_RGB_ORDER TFT_BGR` — CYD panels are wired BGR; removing this swaps red/blue

**include/secrets.h** (gitignored) — WiFi credentials:
- `APP_WIFI_DEFAULT_SSID`
- `APP_WIFI_DEFAULT_PASSWORD`

**NVS Namespace `cyd-core`** — persisted via `include/storage_service.h`:
- Boot count
- Device name (used in WiFiManager portal: `<deviceName>-setup`)
- Debug level (runtime adjustable via `cyd::storage::setDebugLevel()`)

## Architecture & Quirks

**Service pattern**: Every subsystem (`backlight`, `display`, `touch`, `wifi`, `ota`, `rgb_led`, `storage`) exposes `begin()` and `update(nowMs)`, called from `main.cpp` in fixed order.

**Board metadata at runtime**: `cyd::boardProfile()` returns const `BoardProfile` struct (display dims, has touch, has LED). Use this in app code instead of `#ifdef`.

**Touch pipeline**: raw ADC → axis transform (swap/invert per board) → `mapConstrained()` to calibrated screen coords. Touch CS and clock separate (cyd28) vs. shared TFT SPI (cyd40); backend selected via `#if TOUCH_DEDICATED_SPI` in `config.h`.

**WiFi + OTA**: WiFi tries secrets credentials first (blocking, up to `APP_WIFI_CONNECT_TIMEOUT_SEC`), then WiFiManager portal. OTA is armed **only if WiFi is connected when `cyd::ota::begin()` is called**; no runtime re-enable path if WiFi connects later.

**Built-in fonts**: `LOAD_GLCD`, `LOAD_FONT2`–8 active (scaffold diagnostic app). Fork projects should switch to VLW fonts and remove `LOAD_*` defines.

**Backlight**: LEDC channel 0, 5 kHz, 8-bit PWM, core 2.x API (`ledcSetup`/`ledcAttachPin`). Do not use channel 0 in app code; core-3 `ledcAttach` does not build on the pinned platform.

**`loop()` cadence**: `delay(10)` caps main loop to ~100 Hz. Remove/reduce for higher-frequency projects.

**Dead code**: `display_service.cpp` has `drawValidationDemo()` / `updateValidationDemo()` (declared in header, never called by app.cpp). Remove when forking.

**Touch debounce**: `delay(1)` / `delay(2)` in `touch_service.cpp` (cyd40 shared-SPI path) is intentional, not loop bloat.

**Config injection**: TFT_eSPI configured via `-include include/config.h` in `build_flags` — no `User_Setup.h` needed.

**`debugLevel` global**: Declared in `main.cpp`, `extern` in `debug.h`, loaded from NVS on boot.

## Web installer and releases

- Platform pinned to `espressif32@6.12.0`; `partitions_custom.csv` and `PROJECT_NAME` are frozen once released. A project copied from the scaffold renames `PROJECT_NAME` *before* its first release (README "Starting a new project").
- `FIRMWARE_VERSION` / `PROJECT_NAME` stay `#define` – config.h is force-included into C files.
- Release images come only from CI on a `v*` tag on `main`; never publish a local build (it holds `secrets.h`). Never put `firmware-merged.bin` in a manifest.
- Improv is vendored in `lib/ImprovWiFi` – never add it to `lib_deps`. `improvTick()` must run at least every ~1 s (loop and portal loop).

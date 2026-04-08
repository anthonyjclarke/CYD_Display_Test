# CYD Project Scaffold

<!-- Update version badge when FIRMWARE_VERSION changes in include/config.h -->
![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![Platform](https://img.shields.io/badge/platform-ESP32-green.svg)
![PlatformIO](https://img.shields.io/badge/PlatformIO-6.x-orange.svg)
![Board](https://img.shields.io/badge/CYD-2.8%22%20%7C%204.0%22-yellow.svg)
![License](https://img.shields.io/badge/license-MIT-lightgrey.svg)
![Status](https://img.shields.io/badge/status-scaffold-blueviolet.svg)

Reference board pages:

- [LCDWiki 2.8inch ESP32-32E-7789](https://www.lcdwiki.com/2.8inch_ESP32-32E-7789)
- [LCDWiki 4.0inch ESP32-32E Display](https://www.lcdwiki.com/4.0inch_ESP32-32E_Display)

This repository is now a reusable starting scaffold for CYD-based ESP32 projects that target:

- `cyd28`: 2.8-inch CYD (`ESP32-2432S028R`)
- `cyd40`: 4.0-inch LCDWiki ESP32-32E

The goal is one codebase with build-flag board selection, shared core services, and a built-in validation app that proves the hardware baseline before project-specific work starts.

## Confirmed working baseline

The currently verified working baseline is:

- `cyd28` uses `ILI9341`
- `cyd40` uses `ST7796S`
- `TFT_eSPI` is configured through compile-time defines injected with `-include include/config.h`
- `cyd28` touch uses a dedicated SPI bus and `XPT2046_Touchscreen`
- `cyd40` touch uses the shared TFT SPI bus through `TFT_eSPI` raw touch helpers
- both board profiles have working touch calibration values captured on real hardware
- the attempted local `User_Setup.h` / `tft_setup.h` override path is not used in this scaffold

Practical rule:

- keep display configuration in [config.h](include/config.h)
- keep the known-good compile path in [platformio.ini](platformio.ini)

## Scaffold structure

- [config.h](include/config.h): board-select macros, TFT_eSPI setup, pins, touch calibration, and scaffold defaults
- [board_profile.h](include/board_profile.h): C++ board metadata for runtime use
- [debug.h](include/debug.h): leveled serial debug macros
- [storage_service.h](include/storage_service.h): persisted scaffold settings using `Preferences`
- [backlight_service.h](include/backlight_service.h): PWM backlight control
- [rgb_led_service.h](include/rgb_led_service.h): onboard RGB LED support where available
- [touch_service.h](include/touch_service.h): board-abstracted touch state and polling
- [wifi_service.h](include/wifi_service.h): WiFiManager-based connection flow
- [ota_service.h](include/ota_service.h): ArduinoOTA support
- [display_service.h](include/display_service.h): display bootstrap helpers and shared `TFT_eSPI` instance
- [app/app.h](include/app/app.h): minimal project-specific app layer entry point
- [main.cpp](src/main.cpp): scaffold bootstrap and service orchestration

## Included core capabilities

The scaffold currently includes:

- board selection by build flag
- working `TFT_eSPI` display setup for both board profiles
- board-specific display dimensions and GPIO mapping
- touch support for both supported boards, using the correct board-specific backend
- backlight control
- RGB LED support on the 2.8-inch CYD profile
- WiFi management with WiFiManager
- OTA support with ArduinoOTA
- persisted scaffold settings with `Preferences`
- serial debug logging through `debug.h`
- a default full-screen display validation app in `src/app/`
- serial diagnostic output for board, display, WiFi, OTA, and touch state

## Architecture

The current hardware architecture is intentionally split by board:

- display setup is common and always provided by `TFT_eSPI`
- runtime board metadata comes from [board_profile.h](include/board_profile.h)
- compile-time board wiring and calibration live in [config.h](include/config.h)
- touch polling is abstracted behind [touch_service.h](include/touch_service.h), but the backend is board-specific

Touch backend split:

- `cyd28`: dedicated touch SPI bus on `CLK=25`, `MISO=39`, `MOSI=32`, `CS=33`, implemented with `XPT2046_Touchscreen`
- `cyd40`: touch shares the TFT SPI bus on `CLK=14`, `MISO=12`, `MOSI=13`, `CS=33`, implemented with `TFT_eSPI` raw touch access

This split exists because the two boards are not actually wired the same way for touch, even though their TFT wiring is similar.

## Board-specifics currently encoded

### `cyd28`

- reference: [LCDWiki 2.8inch ESP32-32E-7789](https://www.lcdwiki.com/2.8inch_ESP32-32E-7789)
- board name: `ESP32-2432S028R 2.8in`
- driver: `ILI9341`
- landscape view: `320x240`
- backlight: `GPIO21`
- touch bus: dedicated SPI bus `CLK=25`, `MOSI=32`, `MISO=39`, `CS=33`, no IRQ
- touch backend: `XPT2046_Touchscreen`
- touch calibration: `X=200..3800`, `Y=300..3700`
- RGB LED: `R=4`, `G=16`, `B=17`, active low
- LDR: `GPIO34`

### `cyd40`

- reference: [LCDWiki 4.0inch ESP32-32E Display](https://www.lcdwiki.com/4.0inch_ESP32-32E_Display)
- board name: `ESP32-32E 4.0in`
- driver: `ST7796S`
- landscape view: `480x320`
- backlight: `GPIO27`
- TFT SPI/control: `MOSI=13`, `MISO=12`, `SCLK=14`, `CS=15`, `DC=2`, `RST=-1`
- touch shares the TFT SPI pins with `CS=33`, `IRQ=36`
- touch backend: `TFT_eSPI` raw touch polling on the shared bus
- touch calibration: `X=356..3629`, `Y=530..3588`, swapped + raw-inverted before mapping
- no onboard RGB LED pins are currently defined in this scaffold

## Default validation app

The default app is meant to answer “is the platform baseline good?” before project work starts.

On boot, `src/app/app.cpp` draws 5 full-width color bars across the entire active display area and overlays runtime text. This makes resolution, rotation, and touch-state mismatches obvious immediately on both `cyd28` (`320x240`) and `cyd40` (`480x320`).

It validates:

- display init
- full-screen drawing and edge usage
- text rendering
- color output
- runtime board/profile selection
- WiFi status
- OTA enablement
- touch reporting
- boot persistence
- active pin mapping and dimensions in serial diagnostics

## App template layer

Project-specific code should now start in:

- [app/app.h](include/app/app.h)
- [app.cpp](src/app/app.cpp)

The included default app:

- owns the full screen with a deterministic hardware validation layout
- refreshes display status every 200ms and serial diagnostics every 5s
- shows an on-screen checklist: `T` (text pipeline), `B` (display bounds), `W` (WiFi connected), `TOUCH` (touch detected at least once)
- demonstrates use of `cyd::display`, `cyd::network`, `cyd::touch`, `cyd::ota`, `cyd::storage`, and `cyd::boardProfile`

The intent is that future projects replace or extend this module while leaving the platform scaffold intact.

## Build and upload

4-inch board:

```sh
pio run -e cyd40 -t upload
pio device monitor -b 115200
```

2.8-inch board:

```sh
pio run -e cyd28 -t upload
pio device monitor -b 115200
```

## Dependencies

The scaffold currently depends on:

- `bodmer/TFT_eSPI`
- `tzapu/WiFiManager`
- `PaulStoffregen/XPT2046_Touchscreen`
- built-in Arduino ESP32 libraries for `WiFi`, `ArduinoOTA`, `Preferences`, and `SPI`

## Potential optimisations and improvements

These are not bugs — the scaffold works as intended. They are the natural next steps when evolving this into a real project.

### Loop cadence

`loop()` ends with `delay(10)`, which caps execution to approximately 100 iterations per second. For UI-heavy or touch-reactive projects, remove or replace this with `yield()`. The motion demo already self-limits via a 33 ms frame timer, so removing `delay(10)` will not cause runaway redraws.

### OTA re-enable after late WiFi connection

`cyd::ota::begin()` is called once at boot and arms OTA only if WiFi is already connected. If WiFi connects later (e.g., after the captive portal completes), OTA is never enabled. A simple fix is to call `ArduinoOTA.begin()` from inside `network::update()` once `WiFi.isConnected()` becomes true, gated by a flag so it only runs once.

### SPI display frequency

`SPI_FREQUENCY` is currently set to 27 MHz — a conservative value chosen for stability during initial board bring-up. ILI9341 is rated for 40 MHz reads and most boards handle 55 MHz writes reliably. Increasing to 55 MHz will meaningfully improve full-screen redraw throughput for animation-heavy projects. Test for pixel corruption at the higher frequency before shipping.

### Touch calibration UI

Touch calibration values (`TOUCH_X_MIN/MAX`, `TOUCH_Y_MIN/MAX`) are hardcoded per board in `config.h`. These were captured on specific units and may drift across individual panels. A calibration screen (tap corners, store results to NVS via `Preferences`) would make the scaffold portable across units without a recompile.

### Dead code in display_service

`drawValidationDemo()` and `updateValidationDemo()` are declared in `display_service.h` and implemented in `display_service.cpp`, but `app.cpp` draws its own validation layout directly via `tft()` and never calls them. Remove both from the header and implementation when forking to avoid confusion.

### RGB LED scaffold behaviour

`rgb::update()` cycles the LED through red → green → blue on a 1200 ms timer. This is only useful as a visual "alive" indicator during scaffold validation. Real projects should replace this with application-driven colour logic, or call `rgb::setColor()` (currently a file-scope function — promote it to the public header if needed).

### WiFi connection is blocking at boot

`trySecretsWifi()` blocks the main thread for up to `APP_WIFI_CONNECT_TIMEOUT_SEC` seconds using `delay(200)` polling. For projects where fast boot-to-display matters, this can be restructured to a non-blocking attempt with a millis-based timeout, deferring the WiFiManager portal to a later phase.

### Backlight LEDC channel

The backlight service hardcodes LEDC channel 0. If an app adds PWM-driven peripherals (e.g., a buzzer, motor), it must avoid channel 0 or the backlight will be disrupted. Consider making the channel a constant in `config.h` (`APP_BACKLIGHT_LEDC_CHANNEL`) so it can be relocated without editing the service.

### Partition table

The scaffold uses the default Arduino ESP32 partition table, which has no OTA slot. Any project that needs wireless firmware updates must add `partitions_custom.csv` and reference it in `platformio.ini` before the first real flash.

## Notes for future projects

- Keep project-specific code separate from the scaffold services where possible.
- Extend board-specific behavior through the board profile and per-board layout logic rather than scattering raw pin checks through app code.
- If a new CYD variant is added later, start by adding a new profile to [config.h](include/config.h) and [board_profile.h](include/board_profile.h).
- `cyd28` and `cyd40` touch wiring are intentionally different in this scaffold; do not collapse them back to one shared set of touch pins.

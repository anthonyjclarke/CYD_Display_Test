# CYD Project Scaffold

Reference board pages:

- [LCDWiki 2.8inch ESP32-32E-7789](https://www.lcdwiki.com/2.8inch_ESP32-32E-7789)
- [LCDWiki 4.0inch ESP32-32E Display](https://www.lcdwiki.com/4.0inch_ESP32-32E_Display)

This repository is now a reusable starting scaffold for CYD-based ESP32 projects that target:

- `cyd28`: 2.8-inch CYD (`ESP32-2432S028R`)
- `cyd40`: 4.0-inch LCDWiki ESP32-32E

The goal is one codebase with build-flag board selection, shared core services, and a built-in display validation app that proves the hardware baseline before project-specific work starts.

## Confirmed working baseline

The currently verified working display setup is:

- `cyd28` uses `ILI9341`
- `cyd40` uses `ST7796_DRIVER`
- `TFT_eSPI` is configured through compile-time defines injected with `-include include/config.h`
- built-in `TFT_eSPI` text rendering works on the confirmed-good config
- the attempted local `User_Setup.h` / `tft_setup.h` override path is not used in this scaffold

Practical rule:

- keep display configuration in [config.h](/Users/ant/PlatformIO/Projects/CYD_Display_Test/include/config.h)
- keep the known-good compile path in [platformio.ini](/Users/ant/PlatformIO/Projects/CYD_Display_Test/platformio.ini)

## Scaffold structure

- [config.h](/Users/ant/PlatformIO/Projects/CYD_Display_Test/include/config.h): board-select macros, TFT_eSPI setup, pins, touch calibration, and scaffold defaults
- [board_profile.h](/Users/ant/PlatformIO/Projects/CYD_Display_Test/include/board_profile.h): C++ board metadata for runtime use
- [debug.h](/Users/ant/PlatformIO/Projects/CYD_Display_Test/include/debug.h): leveled serial debug macros
- [storage_service.h](/Users/ant/PlatformIO/Projects/CYD_Display_Test/include/storage_service.h): persisted scaffold settings using `Preferences`
- [backlight_service.h](/Users/ant/PlatformIO/Projects/CYD_Display_Test/include/backlight_service.h): PWM backlight control
- [rgb_led_service.h](/Users/ant/PlatformIO/Projects/CYD_Display_Test/include/rgb_led_service.h): onboard RGB LED support where available
- [touch_service.h](/Users/ant/PlatformIO/Projects/CYD_Display_Test/include/touch_service.h): XPT2046 touch handling using board-defined SPI pins
- [wifi_service.h](/Users/ant/PlatformIO/Projects/CYD_Display_Test/include/wifi_service.h): WiFiManager-based connection flow
- [ota_service.h](/Users/ant/PlatformIO/Projects/CYD_Display_Test/include/ota_service.h): ArduinoOTA support
- [display_service.h](/Users/ant/PlatformIO/Projects/CYD_Display_Test/include/display_service.h): display bootstrap helpers and shared `TFT_eSPI` instance
- [app/app.h](/Users/ant/PlatformIO/Projects/CYD_Display_Test/include/app/app.h): minimal project-specific app layer entry point
- [main.cpp](/Users/ant/PlatformIO/Projects/CYD_Display_Test/src/main.cpp): scaffold bootstrap and service orchestration

## Included core capabilities

The scaffold currently includes:

- board selection by build flag
- working `TFT_eSPI` display setup for both board profiles
- board-specific display dimensions and GPIO mapping
- touch support using the configured board pins
- backlight control
- RGB LED support on the 2.8-inch CYD profile
- WiFi management with WiFiManager
- OTA support with ArduinoOTA
- persisted scaffold settings with `Preferences`
- serial debug logging through `debug.h`
- a default full-screen display validation app in `src/app/`
- serial diagnostic output for board, display, WiFi, OTA, and touch state

## Board-specifics currently encoded

### `cyd28`

- reference: [LCDWiki 2.8inch ESP32-32E-7789](https://www.lcdwiki.com/2.8inch_ESP32-32E-7789)
- board name: `ESP32-2432S028R 2.8in`
- driver: `ILI9341`
- landscape view: `320x240`
- backlight: `GPIO21`
- touch bus: `CLK=14`, `MOSI=13`, `MISO=12`, `CS=33`, `IRQ=36`
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
- no onboard RGB LED pins are currently defined in this scaffold

## Default validation app

The default app is meant to answer “is the platform baseline good?” before project work starts.

On boot, `src/app/app.cpp` draws 5 full-width color bars across the entire active display area and overlays runtime text. This makes resolution/rotation mismatches obvious immediately on both `cyd28` (`320x240`) and `cyd40` (`480x320`).

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

- [app/app.h](/Users/ant/PlatformIO/Projects/CYD_Display_Test/include/app/app.h)
- [app.cpp](/Users/ant/PlatformIO/Projects/CYD_Display_Test/src/app/app.cpp)

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
- built-in Arduino ESP32 libraries for `WiFi`, `ArduinoOTA`, `Preferences`, and `SPI`

## Notes for future projects

- Keep project-specific code separate from the scaffold services where possible.
- Extend board-specific behavior through the board profile and per-board layout logic rather than scattering raw pin checks through app code.
- If a new CYD variant is added later, start by adding a new profile to [config.h](/Users/ant/PlatformIO/Projects/CYD_Display_Test/include/config.h) and [board_profile.h](/Users/ant/PlatformIO/Projects/CYD_Display_Test/include/board_profile.h).
- The 4-inch touch calibration values are still generic starting values and may need refinement on real hardware.

#pragma once

// Select the board profile in platformio.ini with:
//   -D CYD_BOARD_28
// or
//   -D CYD_BOARD_40
//
// If nothing is selected, default to the 2.8-inch CYD profile.

#if !defined(CYD_BOARD_28) && !defined(CYD_BOARD_40)
#define CYD_BOARD_28
#endif

// Common control pins
#define TFT_CS_PIN    15
#define TFT_DC_PIN    2
#define TFT_MOSI_PIN  13
#define TFT_SCLK_PIN  14
#define TFT_MISO_PIN  12
#define TFT_RST_PIN   -1

#define TOUCH_CS_PIN   33
#define TOUCH_IRQ_PIN  36
#define TOUCH_MOSI_PIN 13
#define TOUCH_MISO_PIN 12
#define TOUCH_CLK_PIN  14

#if defined(CYD_BOARD_28)
#define CYD_BOARD_NAME       "ESP32-2432S028R 2.8in"
#define CYD_BOARD_KEY        "cyd28"
#define CYD_DRIVER_NAME      "ILI9341"
#define CYD_NATIVE_WIDTH     240
#define CYD_NATIVE_HEIGHT    320
#define CYD_LANDSCAPE_ROT    1
#define TFT_BL_PIN           21
#define CYD_EXPECTED_WIDTH   320
#define CYD_EXPECTED_HEIGHT  240
#define CYD_DRIVER_ILI9341
#define CYD_HAS_RGB_LED      1
#define CYD_HAS_LDR          1
#define LED_R_PIN            4
#define LED_G_PIN            16
#define LED_B_PIN            17
#define CYD_RGB_ACTIVE_LOW   1
#define LDR_PIN              34
#define TOUCH_X_MIN          200
#define TOUCH_X_MAX          3800
#define TOUCH_Y_MIN          300
#define TOUCH_Y_MAX          3700
#define TOUCH_INVERT_X       0
#define TOUCH_INVERT_Y       0
#endif

#if defined(CYD_BOARD_40)
#define CYD_BOARD_NAME       "ESP32-32E 4.0in"
#define CYD_BOARD_KEY        "cyd40"
#define CYD_DRIVER_NAME      "ST7796S"
#define CYD_NATIVE_WIDTH     320
#define CYD_NATIVE_HEIGHT    480
#define CYD_LANDSCAPE_ROT    1
#define TFT_BL_PIN           27
#define CYD_EXPECTED_WIDTH   480
#define CYD_EXPECTED_HEIGHT  320
#define CYD_DRIVER_ST7796
#define CYD_HAS_RGB_LED      0
#define CYD_HAS_LDR          0
#define LED_R_PIN            -1
#define LED_G_PIN            -1
#define LED_B_PIN            -1
#define CYD_RGB_ACTIVE_LOW   0
#define LDR_PIN              -1
#define TOUCH_X_MIN          200
#define TOUCH_X_MAX          3900
#define TOUCH_Y_MIN          200
#define TOUCH_Y_MAX          3900
#define TOUCH_INVERT_X       1
#define TOUCH_INVERT_Y       1
#endif

// TFT_eSPI compile-time configuration.
// These macros are consumed when TFT_eSPI.h is included.
#define USER_SETUP_LOADED

// Enable the built-in fonts used by the diagnostic sketch.
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8

#ifdef CYD_DRIVER_ILI9341
#define ILI9341_DRIVER
#endif

#ifdef CYD_DRIVER_ST7796
#define ST7796_DRIVER
#endif

#define TFT_MOSI  TFT_MOSI_PIN
#define TFT_MISO  TFT_MISO_PIN
#define TFT_SCLK  TFT_SCLK_PIN
#define TFT_CS    TFT_CS_PIN
#define TFT_DC    TFT_DC_PIN
#define TFT_RST   TFT_RST_PIN
#define TOUCH_CS  TOUCH_CS_PIN

// CYD panels are commonly wired BGR. If colours are swapped, remove this define.
#define TFT_RGB_ORDER TFT_BGR

#define APP_DEFAULT_DEBUG_LEVEL      3
#define APP_DEFAULT_BRIGHTNESS       255
#define APP_WIFI_PORTAL_TIMEOUT_SEC  180
#define APP_WIFI_CONNECT_TIMEOUT_SEC 20
#define APP_OTA_PORT                 3232

#define SPI_FREQUENCY       27000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY 2500000

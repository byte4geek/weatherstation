# ESP32 port — byte4geek WeatherStation

This is a **source-level compatibility port**, NOT a validated release. It has **not been compiled with PlatformIO in this environment**, nor tested on physical hardware. Do not upload until build errors have been resolved and the GPIO map and wiring have been checked.

## How to build on Windows / VS Code

1. Extract ZIP, open the inner `weatherstation-main` folder in VS Code (it contains `platformio.ini`).
2. PlatformIO > Project Tasks > **esp32dev** > General > **Build**. If the build fails, send the FIRST compiler error and surrounding lines to ChatGPT.
3. Do not use the precompiled `WeatherStation_*.bin` files from the upstream repository: they are intended for ESP8266.
4. Only after a successful build and pin/wiring review: connect ESP32 on COM9, select `esp32dev` > Upload, then monitor at 115200 baud. COM9 is the prior connection on the user's PC, not fixed in this configuration.

## Changes

- Added `esp32dev` PlatformIO environment with separate dependencies (ESP32 built-in Preferences, no ESP8266 EspSaveCrash).
- ESP32/ESP8266 conditional WiFi, HTTP, web-server, TLS-client headers.
- ESP32 uses `WebServer`, `WiFiClientSecure`, `WiFi.setHostname()`, `WiFi.setSleep(false)`, `esp_random()`, and an ESP32 chip identifier for MQTT.
- ESP8266-only SDK DHCP operations, Wi-Fi event handlers, ADC_VCC, and EspSaveCrash calls are excluded from ESP32 build. Crash reporting is **not implemented on ESP32**.
- ESP32 returns a reset-reason numeric code; `heap_fragmentation` placeholder is 0; `vcc` is not measured (NaN). These metrics do not have direct ESP32 equivalents.
- OTA upload uses UPDATE_SIZE_UNKNOWN; requires real hardware verification and sufficient partition space.

## Hardware review required

Upstream default pins are **ESP8266 settings**, not verified ESP32 wiring: GPIO14 rain, GPIO12 wind, GPIO4 SDA, GPIO5 SCL, GPIO0 factory reset, GPIO2 LED. **GPIO12 and GPIO0 are ESP32 strapping pins** and external circuits can prevent boot. GPIO2 may or may not have an onboard LED on your ELEGOO board. Check the exact ESP32 board model and wiring, and adjust GPIO assignments before connecting sensors. Do not attach 5V signals to ESP32 inputs; the GPIOs are 3.3V logic.

The original firmware has an outbound device telemetry/ping feature and some uploaders use `setInsecure()` for HTTPS. Review those behaviors and settings before connecting it to a network or exposing its web interface. Do not expose the device's web interface directly to the public internet.

If the ESP32 build fails, collect the earliest error from PlatformIO. Changes to the web server, OTA API, WiFiManager, and third-party sensor libraries may still be needed for your installed Arduino ESP32 core version.

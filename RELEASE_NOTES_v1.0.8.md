# Release v1.0.8 — Multi-Platform ESP8266 & ESP32 Support, Mesh Wi-Fi Resilience & Uptime Diagnostics Fix

We are excited to announce **Firmware Release v1.0.8**! This release unifies the codebase to natively support both **ESP8266** (NodeMCU v2/v3, ESP-12E) and **ESP32** (ESP32-WROOM-32, DevKitC, NodeMCU-32S) architectures from a single source, resolves Wi-Fi drops on Mesh networks, enhances OTA update mechanisms, and fixes Cloudflare ping uptime reporting.

---

## 🚀 What's New in v1.0.8

### 1. ⚡ Cross-Platform Microcontroller Support (ESP8266 & ESP32)
* **Unified Codebase**: Compile the exact same source code for either ESP8266 or ESP32 by selecting the PlatformIO environment (`nodemcuv2` or `esp32dev`).
* **Zero-Overhead Hardware Abstraction (`esp_compat.h`)**:
  * Cross-platform CPU clock detection (`weather_cpu_frequency_mhz()`).
  * Microcontroller-aware reset reasons and diagnostic strings (`weather_reset_reason()`).
  * Board-specific onboard LED polarities (GPIO 2 active-LOW on ESP8266, active-HIGH on ESP32).
  * Safe IRAM-resident GPIO interrupt handling on ESP32 (`esp_timer_get_time()` and native `gpio_install_isr_service(ESP_INTR_FLAG_IRAM)`), preventing `IllegalInstruction` / Flash cache crashes during concurrent NVS writes.
* **Dual OTA Partitions on ESP32**: Configured with `min_spiffs.csv` to provide two 1.92 MB OTA slots (`ota_0` and `ota_1`), enabling seamless in-browser web updates.
* **Smart OTA Web Handler**: Automatically detects whether an uploaded binary is an application OTA image (`.bin`) or a full merged `0x0` flash image, seamlessly skipping the 64 KB header when needed.
* **Dynamic Web UI Labels**: Web dashboard header, silkscreen pin mappings, and wiring tables automatically adjust labels (`D0-D8` vs `D<gpio>`) based on the active microcontroller family.

### 2. 📶 Wi-Fi Mesh & Auto-Channel Roaming Resilience
* **Multi-Channel Scanning (`WIFI_ALL_CHANNEL_SCAN`)**: Solves disconnections (`BEACON_TIMEOUT` / Reason Code 200) when dynamic Mesh access points or auto-channel routers hop between channels 1, 6, and 11.
* **Best-Signal AP Association (`WIFI_CONNECT_AP_BY_SIGNAL`)**: Always binds to the mesh node with the strongest RSSI.
* **Non-Blocking Wi-Fi Watchdog**: Continuously monitors station status and automatically triggers a full-band reconnection if the link drops for more than 12 seconds.
* **Precise SDK Diagnostic Events**: Real-time event callbacks log Wi-Fi association, BSSID, channel changes, and DHCP IP assignments on both platforms.

### 3. ☁️ Cloudflare Ping & Uptime Fixes
* **Uptime Field Compatibility**: Populates both `uptime` (legacy schema) and `uptime_s` (modern schema) in JSON ping payloads, fixing the issue where uptime showed as `0` in Cloudflare D1 databases.
* **Unblocked Boot Ping**: Boot and reboot telemetry pings fire reliably 8 seconds after Wi-Fi association, even on devices where MQTT is unconfigured.
* **Hardware-Tagged UUIDs**: Device identifiers now carry hardware prefixes (`ESP32_<hex>` or `ESP8266_<hex>`) with automatic in-place migration of legacy `ws_<hex>` IDs.
* **Clean Console Output**: Silenced automatic background ping notices in the debug log buffer.
* **New Hidden Diagnostic Command**: Added the console command `send_alive` to trigger an immediate Cloudflare telemetry check with visible HTTP response logging.

---

## 🛠️ Pre-Compiled Firmware Binaries

| Binary File | Target Hardware | Recommended Flash Method |
|---|---|---|
| **`WeatherStation_v1.0.8_ESP8266.bin`** | ESP8266 NodeMCU v2/v3 / ESP-12E | Web UI OTA (`/update`) or USB via esptool |
| **`WeatherStation_v1.0.8_ESP32_OTA.bin`** | ESP32-WROOM-32 / DevKitC | Web UI OTA (`/update`) |
| **`WeatherStation_v1.0.8_ESP32_MERGED_0x0.bin`** | ESP32-WROOM-32 / DevKitC | Fresh USB flash at offset `0x0` via Espressif Flash Download Tool or esptool.py |

---

## 💻 Compiling with PlatformIO

```bash
# Compile for ESP8266
pio run -e nodemcuv2

# Compile for ESP32
pio run -e esp32dev

# Compile both architectures simultaneously
pio run
```

---

## 📦 Flashing a Fresh ESP32 at Offset `0x0`

If flashing an ESP32 for the first time via USB:
```bash
esptool.py --chip esp32 write_flash -z 0x0 WeatherStation_v1.0.8_ESP32_MERGED_0x0.bin
```
Or use the **Espressif Flash Download Tool** at address `0x0` with SPI Speed `40MHz` and SPI Mode `DIO`.

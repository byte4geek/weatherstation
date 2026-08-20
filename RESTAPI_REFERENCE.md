# 📡 REST API Reference & Integration Guide

This document provides complete documentation for the REST API endpoints exposed by the **DIY Smart Weather Station (ESP8266)**. You can use these endpoints to query real-time sensor measurements, fetch and modify configuration parameters, trigger sensor calibration wizards, execute console commands, download system backups, or integrate the weather station into third-party automation systems (Home Assistant, Node-RED, Python scripts, openHAB, Domoticz, etc.).

---

## 📋 Endpoint Summary Table

The weather station exposes **15 HTTP endpoints**:

| Method | Endpoint | Description | Response Content-Type |
|---|---|---|---|
| `GET` | `/api/status` | Read real-time weather readings & system health | `application/json` |
| `GET` | `/api/config` | Read active settings & calibration factors | `application/json` |
| `GET` | `/api/backup` | Download full NVS configuration backup (includes Wi-Fi keys) | `application/json` |
| `POST` | `/api/save_config` | Update settings and save to NVS flash | `text/plain` |
| `POST` | `/api/clear_counters` | Reset total rain counter and 24h history buffer | `text/plain` |
| `POST` | `/api/calibrate_north` | Calibrate wind direction vane to North ($0^\circ$) | `application/json` |
| `POST` | `/api/calibrate_lux_unfiltered` | Lux PETG Wizard: Step 1 (Unfiltered reference) | `application/json` |
| `POST` | `/api/calibrate_lux_filtered` | Lux PETG Wizard: Step 2 (Filtered reading & transmission factor) | `application/json` |
| `GET` | `/api/logs` | Fetch live ring-buffer debug console logs | `application/json` |
| `POST` | `/api/command` | Execute interactive console commands | `text/plain` |
| `POST` | `/api/ota` | Upload new firmware binary (`.bin`) via OTA | `text/plain` |
| `POST` | `/api/reboot` | Soft restart ESP8266 microcontroller | `application/json` |
| `POST` | `/api/factory_reset` | Wipe NVS memory & reboot into Access Point Setup mode | `text/plain` |
| `GET` | `/api/crash` | Diagnostic endpoint: triggers hardware store exception | `application/json` |
| `GET` | `/` | Web Dashboard HTML user interface | `text/html` |

---

## 🔑 Headers & Data Formats

- **Base URL**: `http://<IP_ADDRESS>` or `http://<HOSTNAME>.local` (default: `http://192.168.1.150` or `http://WeatherStation.local`)
- **JSON Requests**: Send `Content-Type: application/json` for POST endpoints expecting JSON bodies.
- **Form-Encoded Requests**: Send `Content-Type: application/x-www-form-urlencoded` for `/api/command`.

---

## 1. 📊 Real-Time Status Endpoint

### `GET /api/status`

Returns all current sensor readings, rain totals, wind speed metrics (multi-unit), air quality ratings, sensor detection flags, and system hardware status.

#### Sample Response (`200 OK`)
```json
{
  "use_imperial": false,
  "ui_compact": false,
  "tips": 12,
  "total_rain": 7.58,
  "hourly_rain": 1.26,
  "daily_rain": 3.79,
  "is_raining": false,
  "wind_speed": 12.4,
  "wind_gust": 21.8,
  "wind_speed_ms": 3.44,
  "wind_speed_kt": 6.70,
  "wind_speed_mph": 7.71,
  "wind_gust_ms": 6.06,
  "wind_gust_kt": 11.77,
  "wind_gust_mph": 13.55,
  "has_aht20": true,
  "has_bmp280": true,
  "has_ens160": true,
  "has_as5600": true,
  "has_bh1750": true,
  "lux": 1450.0,
  "wind_dir": 315.0,
  "temp": 24.5,
  "hum": 58.2,
  "press": 1013.25,
  "tvoc": 45,
  "eco2": 420,
  "aqi": 1,
  "uptime": "01d, 04:12:35",
  "ssid": "HOME-WIFI",
  "time": "2026-08-07 11:15:00",
  "rssi": "-62 dBm",
  "ip": "192.168.1.150",
  "vcc": 3.28,
  "heap": 34816
}
```

#### Field Explanations:
- `use_imperial`: `true` if Imperial units (°F, mph, in, inHg) are enabled, `false` for Metric (°C, km/h, mm, hPa).
- `ui_compact`: `true` if Compact Board UI mode is active.
- `total_rain`: Cumulative total rain accumulated since last reset (mm or inches).
- `hourly_rain`: Rolling rain total over the last 60 minutes.
- `daily_rain`: Rolling rain total over the last 24 hours (1440-minute sliding window).
- `is_raining`: `true` if rainfall was registered within the last 5 minutes.
- `wind_speed` / `wind_gust`: Current wind speed and peak gust today (km/h or mph depending on unit setting).
- `wind_speed_ms`, `wind_speed_kt`, `wind_speed_mph`: Pre-calculated speed conversions in meters/sec, knots, and miles/hour.
- `has_*`: Boolean flags indicating if each sensor was detected on the I2C / GPIO bus.
- `temp`, `hum`, `press`, `lux`, `wind_dir`: Current readings from AHT20/BMP280/BH1750/AS5600 sensors (returns `null` if sensor is offline).
- `tvoc`, `eco2`, `aqi`: Air quality measurements from ENS160 (returns `null` if ENS160 is offline). `aqi` ranges 1 (Excellent) to 5 (Unhealthy).
- `vcc`: Microcontroller operating voltage (V).
- `heap`: Free RAM memory in bytes.

---

## 2. ⚙️ Configuration Endpoints

### `GET /api/config`

Returns current configuration settings, I2C pin assignments, calibration constants, network settings, and MQTT parameters.

#### Sample Response (`200 OK`)
```json
{
  "fw_version": "1.0.4",
  "hostname": "WeatherStation",
  "use_imperial": false,
  "ui_compact": false,
  "tz_off": 1,
  "use_dst": false,
  "pin": 14,
  "calibration": 0.6314,
  "debounce": 300,
  "dhcp": true,
  "crash_opt": true,
  "sda": 4,
  "scl": 5,
  "i2c_clk": 100,
  "altitude": 120,
  "t_offset": 0.0,
  "h_offset": 0.0,
  "p_offset": 0.0,
  "w_pin": 12,
  "w_rad": 80,
  "w_mag": 1,
  "w_fac": 3.0,
  "w_cal": 5.4287,
  "w_deb": 15,
  "w_dir_off": 45,
  "w_spd_int": 2,
  "w_spd_avg": 5,
  "w_dir_avg": 5,
  "lux_cal": 0.8500,
  "mqtt_int": 15,
  "sens_int": 5,
  "mqtt_dec": 1,
  "ip": "192.168.1.150",
  "gw": "192.168.1.1",
  "nm": "255.255.255.0",
  "dns_p": "192.168.1.1",
  "dns_s": "8.8.8.8",
  "ntp": "pool.ntp.org",
  "mqtt": "192.168.1.50",
  "m_port": 1883,
  "m_user": "homeassistant",
  "m_pass": "secret"
}
```

---

### `GET /api/backup`

Returns a full configuration backup payload in JSON format, **including saved Wi-Fi credentials** (`wifi_ssid` and `wifi_pass`), suitable for exporting to a local file and restoring later.

---

### `POST /api/save_config`

Saves new configuration values to NVS flash memory and restarts the ESP8266 to apply changes.

#### Content-Type: `application/json`

#### Example Body:
```json
{
  "hostname": "WeatherStation",
  "use_imperial": false,
  "ui_compact": true,
  "tz_off": 1,
  "use_dst": true,
  "pin": 14,
  "calibration": 0.6314,
  "debounce": 300,
  "dhcp": true,
  "sda": 4,
  "scl": 5,
  "altitude": 120,
  "w_pin": 12,
  "w_rad": 80,
  "w_mag": 1,
  "w_fac": 3.0,
  "w_deb": 15,
  "w_dir_off": 45,
  "w_spd_int": 2,
  "w_spd_avg": 5,
  "w_dir_avg": 5,
  "lux_cal": 0.85,
  "ntp": "pool.ntp.org",
  "mqtt": "192.168.1.50",
  "m_port": 1883,
  "m_user": "homeassistant",
  "m_pass": "secret",
  "mqtt_int": 15,
  "sens_int": 5,
  "mqtt_dec": 1
}
```

#### Response: `200 OK` (Body: `OK`)

---

## 3. 🎯 Sensor Calibration & Maintenance Endpoints

### `POST /api/clear_counters`

Resets the cumulative rain tips counter (`total_bucket_tips`), sets `total_rain_mm = 0.0`, and clears the 24-hour sliding window history buffer.

#### Response: `200 OK` (Body: `OK`)

---

### `POST /api/calibrate_north`

Calibrates the magnetic wind vane direction sensor (AS5600). The current physical position of the wind vane is set as **North ($0^\circ$)**, calculating and saving the offset angle to NVS flash.

#### Response (`200 OK`):
```json
{
  "status": "success",
  "offset": 142
}
```

---

### `POST /api/calibrate_lux_unfiltered`

**Lux Sensor PETG Cover Calibration Wizard - Step 1**: Measure unfiltered reference light intensity without the PETG housing cover.

#### Response (`200 OK`):
```json
{
  "status": "success",
  "unfiltered_lux": 1520.4
}
```

---

### `POST /api/calibrate_lux_filtered`

**Lux Sensor PETG Cover Calibration Wizard - Step 2**: Measure light intensity with the PETG cover installed under the same light source. Computes the transmission ratio factor ($\text{filtered} / \text{unfiltered}$) and saves `lux_cal_factor` to NVS flash.

#### Response (`200 OK`):
```json
{
  "status": "success",
  "factor": 0.8421,
  "transmission_pct": 84.2
}
```

---

## 4. 💻 Debug Console & System Control Endpoints

### `GET /api/logs`

Returns an array of strings representing the last 60 entries stored in the microcontroller's RAM ring-buffer log.

#### Sample Response (`200 OK`):
```json
[
  "[2026-08-07 11:10:02] [WiFi SUCCESS] Connected to 'HOME-WIFI'! IP: 192.168.1.150 | RSSI: -62 dBm",
  "[2026-08-07 11:10:03] NTP time sync updated (GMT +1, DST: Disabled).",
  "[2026-08-07 11:10:05] [Rain] Tip detected! Incremented by 1, Total tips: 12, Total rain: 7.58 mm",
  "[2026-08-07 11:11:00] [System] Minute ticked. History index: 15 | Hourly rain: 1.26 mm | 24h rain: 3.79 mm"
]
```

---

### `POST /api/command`

Executes an interactive debug console command.

#### Content-Type: `application/x-www-form-urlencoded`
#### Form Field: `cmd=<COMMAND_STRING>`

#### Supported Commands:
- `help`: Returns list of available commands.
- `status`: Logs detailed sensor and memory state to console.
- `clear_logs`: Clears the RAM log ring buffer.
- `i2c_scan`: Scans I2C bus (addresses 0x01-0x7F) and prints detected addresses (`0x23` BH1750, `0x36` AS5600, `0x38` AHT20, `0x52`/`0x53` ENS160, `0x76` BMP280).
- `clear_rain`: Resets total rain tips counter.
- `clear_gust`: Resets peak wind gust recorded today back to 0.0 km/h.
- `calibrate_north`: Sets current AS5600 position as North.
- `reset_ens160`: Resets the ENS160 internal baseline engine and re-initializes air quality measurements.
- `loglevel debug` / `loglevel info`: Enables/disables verbose I2C bus traffic logging.
- `set_cal <val>`: Sets rain gauge calibration factor (mm/tip).
- `set_deb <val>`: Sets rain gauge debounce duration in ms.
- `set_pin <val>`: Sets rain gauge GPIO pin.

#### Response (`200 OK`): `OK` or command output text.

---

### `POST /api/reboot`

Triggers a soft restart of the ESP8266 microcontroller.

#### Response (`200 OK`):
```json
{
  "status": "success",
  "message": "Rebooting..."
}
```

---

### `POST /api/factory_reset`

Erases all stored preferences from NVS flash memory (Wi-Fi credentials, MQTT configuration, offsets, and calibrations) and restarts the ESP8266 in **Access Point setup mode** (`WeatherStation_Setup`).

#### Response (`200 OK`): `OK`

---

### `POST /api/ota`

Over-The-Air firmware update endpoint. Accepts binary multipart firmware uploads (`.bin` files compiled via PlatformIO).

#### Response (`200 OK`): `OK` (or `FAIL` if flash update failed).

---

### `GET /api/crash`

**Diagnostic/Test Endpoint**: Intentionally dereferences a `nullptr` to trigger a hardware `StoreProhibited` exception. Used for testing ESP8266 stack trace crash dump collection (`EspSaveCrash`).

---

## 💻 Integration Code Examples

### 1. cURL (Command Line)

#### Get Current Weather Status:
```bash
curl -X GET http://192.168.1.150/api/status
```

#### Reset Rain Counter:
```bash
curl -X POST http://192.168.1.150/api/clear_counters
```

#### Execute Console Command (I2C Bus Scan):
```bash
curl -X POST http://192.168.1.150/api/command -d "cmd=i2c_scan"
```

---

### 2. Python 3 Script

```python
import requests

IP = "192.168.1.150"

# Read sensor status
res = requests.get(f"http://{IP}/api/status")
data = res.json()

print(f"Temperature: {data['temp']} °C")
print(f"Humidity: {data['hum']} %")
print(f"Pressure: {data['press']} hPa")
print(f"Wind Speed: {data['wind_speed']} km/h ({data['wind_speed_ms']} m/s, {data['wind_speed_kt']} kt)")
print(f"Rain (24h): {data['daily_rain']} mm")
print(f"Air Quality (AQI): {data['aqi']}")
```

---

### 3. Home Assistant `configuration.yaml` (REST Sensors)

If you prefer using REST polling instead of MQTT Auto-Discovery:

```yaml
sensor:
  - platform: rest
    name: "Weather Station Temperature"
    resource: http://192.168.1.150/api/status
    value_template: "{{ value_json.temp }}"
    unit_of_measurement: "°C"
    device_class: temperature
    scan_interval: 10

  - platform: rest
    name: "Weather Station Wind Speed"
    resource: http://192.168.1.150/api/status
    value_template: "{{ value_json.wind_speed }}"
    unit_of_measurement: "km/h"
    device_class: wind_speed
    scan_interval: 5

rest_command:
  reset_weather_rain:
    url: "http://192.168.1.150/api/clear_counters"
    method: post
```

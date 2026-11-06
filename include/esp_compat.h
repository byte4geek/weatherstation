#pragma once
#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#include <esp_system.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
inline uint32_t weather_cpu_frequency_mhz() { return getCpuFrequencyMhz(); }
inline String weather_reset_reason() { return String((int)esp_reset_reason()); }
inline String weather_reset_info() { return weather_reset_reason(); }
inline int weather_heap_fragmentation() { return 0; } // Not exposed on ESP32; placeholder
inline String weather_firmware_md5() { return ESP.getSketchMD5(); }
inline float weather_supply_voltage() { return NAN; } // Vcc not measurable via ESP.getVcc() on ESP32
inline String weather_chip_id() { return String((uint32_t)ESP.getEfuseMac(), HEX); }
inline void weather_set_hostname(const String& hostname) { WiFi.setHostname(hostname.c_str()); }
inline void weather_disable_wifi_sleep() { WiFi.setSleep(false); }
#else
#include <ESP8266WiFi.h>
inline uint32_t weather_cpu_frequency_mhz() { return ESP.getCpuFreqMHz(); }
inline String weather_reset_reason() { return ESP.getResetReason(); }
inline String weather_reset_info() { return ESP.getResetInfo(); }
inline int weather_heap_fragmentation() { return ESP.getHeapFragmentation(); }
inline String weather_firmware_md5() { return ESP.getSketchMD5(); }
inline float weather_supply_voltage() { return ESP.getVcc() / 1000.0f; }
inline String weather_chip_id() { return String(ESP.getChipId(), HEX); }
inline void weather_set_hostname(const String& hostname) { WiFi.hostname(hostname); }
inline void weather_disable_wifi_sleep() { WiFi.setSleepMode(WIFI_NONE_SLEEP); }
#endif

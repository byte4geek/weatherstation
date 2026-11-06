// File: include/esp_compat.h
// Cross-platform hardware abstraction layer for ESP8266 (NodeMCU v2 / ESP-12E)
// and ESP32 (ESP32-WROOM-32 / ESP32-DevKitC / NodeMCU-32S).
#pragma once

#include <Arduino.h>

#if defined(ESP32)
#include <WiFi.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <driver/gpio.h>

// Human-readable microcontroller family name used in logs and Web UI
#define WEATHER_MCU_NAME "ESP32"

// On ESP32-WROOM / DevKit boards, the onboard blue LED on GPIO 2 is active-HIGH (HIGH = ON, LOW = OFF)
#define WEATHER_LED_ON  HIGH
#define WEATHER_LED_OFF LOW

// Returns the current CPU clock frequency in MHz
inline uint32_t weather_cpu_frequency_mhz() {
    return getCpuFrequencyMhz();
}

// Translates ESP-IDF reset reason enum into a descriptive human-readable string
inline String weather_reset_reason() {
    switch (esp_reset_reason()) {
        case ESP_RST_POWERON:   return "Power-on reset";
        case ESP_RST_EXT:       return "External pin reset";
        case ESP_RST_SW:        return "Software/System restart";
        case ESP_RST_PANIC:     return "Exception/Panic reset";
        case ESP_RST_INT_WDT:   return "Interrupt watchdog reset";
        case ESP_RST_TASK_WDT:  return "Task watchdog reset";
        case ESP_RST_WDT:       return "Other watchdog reset";
        case ESP_RST_DEEPSLEEP: return "Deep-sleep wakeup";
        case ESP_RST_BROWNOUT:  return "Brownout reset";
        case ESP_RST_SDIO:      return "SDIO reset";
        default:                return String("Unknown (") + String((int)esp_reset_reason()) + ")";
    }
}

// Returns detailed reset info (mirrors weather_reset_reason on ESP32)
inline String weather_reset_info() {
    return weather_reset_reason();
}

// Heap fragmentation percentage (not directly exposed by ESP32 Arduino core; returns 0)
inline int weather_heap_fragmentation() {
    return 0;
}

// Returns the MD5 checksum of the currently running firmware image
inline String weather_firmware_md5() {
    return ESP.getSketchMD5();
}

// Supply voltage in Volts (internal ADC_VCC measurement is only supported on ESP8266; returns NAN on ESP32)
inline float weather_supply_voltage() {
    return NAN;
}

// Returns a unique hexadecimal chip identifier derived from the eFuse MAC address
inline String weather_chip_id() {
    return String((uint32_t)ESP.getEfuseMac(), HEX);
}

// Sets the network station hostname prior to or during Wi-Fi connection
inline void weather_set_hostname(const String& hostname) {
    WiFi.setHostname(hostname.c_str());
}

// Disables Wi-Fi modem power-saving sleep to improve 4-way handshake and HTTP server responsiveness
inline void weather_disable_wifi_sleep() {
    WiFi.setSleep(false);
}

// IRAM-safe microsecond timestamp for GPIO ISRs on ESP32.
// Arduino's millis() and __onPinInterrupt live in Flash on ESP32 and cause an IllegalInstruction
// panic if a GPIO interrupt fires while NVS Flash writes temporarily disable the Flash cache.
// esp_timer_get_time() resides in IRAM (0x4008xxxx) and is safe to call while Flash cache is disabled.
static inline uint32_t IRAM_ATTR weather_isr_micros() {
    return (uint32_t)esp_timer_get_time();
}

// Registers a falling-edge GPIO interrupt handler directly with the ESP-IDF IRAM ISR service,
// bypassing Arduino-ESP32's Flash-resident __onPinInterrupt wrapper.
inline void weather_attach_falling_isr(int pin, void (*isr_fn)(void*)) {
    pinMode(pin, INPUT_PULLUP);
    // Install global GPIO ISR service in IRAM if not already installed
    static bool isr_service_installed = false;
    if (!isr_service_installed) {
        esp_err_t err = gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
        if (err == ESP_OK || err == ESP_ERR_INVALID_STATE) {
            isr_service_installed = true;
        }
    }
    gpio_num_t gpio = (gpio_num_t)pin;
    gpio_set_intr_type(gpio, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_remove(gpio);
    gpio_isr_handler_add(gpio, isr_fn, nullptr);
    gpio_intr_enable(gpio);
}

#else // ESP8266 (NodeMCU v2 / ESP-12E)

#include <ESP8266WiFi.h>

// Human-readable microcontroller family name used in logs and Web UI
#define WEATHER_MCU_NAME "ESP8266"

// On ESP8266 NodeMCU / ESP-12E, the onboard blue LED on GPIO 2 is active-LOW (LOW = ON, HIGH = OFF)
#define WEATHER_LED_ON  LOW
#define WEATHER_LED_OFF HIGH

// Returns the current CPU clock frequency in MHz
inline uint32_t weather_cpu_frequency_mhz() {
    return ESP.getCpuFreqMHz();
}

// Returns a human-readable string describing the last reset cause
inline String weather_reset_reason() {
    return ESP.getResetReason();
}

// Returns detailed SDK reset and exception information
inline String weather_reset_info() {
    return ESP.getResetInfo();
}

// Returns current heap fragmentation percentage (0-100%)
inline int weather_heap_fragmentation() {
    return ESP.getHeapFragmentation();
}

// Returns the MD5 checksum of the currently running firmware sketch
inline String weather_firmware_md5() {
    return ESP.getSketchMD5();
}

// Returns internal supply voltage (VCC) in Volts measured via ADC_MODE(ADC_VCC)
inline float weather_supply_voltage() {
    return ESP.getVcc() / 1000.0f;
}

// Returns the unique 32-bit ESP8266 Chip ID in hexadecimal format
inline String weather_chip_id() {
    return String(ESP.getChipId(), HEX);
}

// Sets the network station hostname on ESP8266
inline void weather_set_hostname(const String& hostname) {
    WiFi.hostname(hostname);
}

// Disables Wi-Fi modem sleep to prevent dropped packets during WPA2 4-way handshake
inline void weather_disable_wifi_sleep() {
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
}

#endif

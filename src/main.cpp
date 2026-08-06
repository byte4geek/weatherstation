// File: src/main.cpp
#include "globals.h"
#include <Ticker.h>

#include <EspSaveCrash.h>


#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
extern "C" {
  #include "user_interface.h"
}

bool is_reboot_pending = false;

void request_reboot() {
    prefs.begin("weather", false);
    prefs.putBool("reboot_pending", true);
    prefs.end();
    delay(500);
    ESP.restart();
}

String get_ping_device_id() {
    prefs.begin("weather", false);
    String id = prefs.getString("device_id", "");
    if (id == "") {
        // Generate a random ID: ws_ followed by two 32-bit random hex values
        id = "ws_" + String(os_random(), HEX) + String(os_random(), HEX);
        prefs.putString("device_id", id);

    }
    prefs.end();
    return id;
}

// Ping endpoint and authorization token XOR'
const uint8_t URL[] = {50, 46, 46, 42, 41, 96, 117, 117, 45, 63, 59, 46, 50, 63, 40, 119, 41, 46, 59, 46, 51, 53, 52, 116, 56, 35, 46, 63, 110, 61, 63, 63, 49, 116, 45, 53, 40, 49, 63, 40, 41, 116, 62, 63, 44, 117, 59, 42, 51, 117, 63, 44, 63, 52, 46};
const size_t URL_LEN = sizeof(URL);
const uint8_t TOKEN[] = {9, 31, 25, 8, 31, 14, 5, 13, 9, 5, 14, 21, 17, 31, 20, 5, 104, 106, 104, 108};
const size_t TOKEN_LEN = sizeof(TOKEN);

String xor_decrypt(const uint8_t *data, size_t len, uint8_t key_val) {
    String decrypted = "";
    for (size_t i = 0; i < len; i++) {
        decrypted += (char)(data[i] ^ key_val);
    }
    return decrypted;
}

void send_cloud_ping(String event_type) {
    if (WiFi.status() != WL_CONNECTED) return;
    
    WiFiClientSecure client;
    client.setInsecure(); // Bypass certificate check for simplicity
    client.setBufferSizes(1024, 1024); // Reduce TLS buffer size to avoid heap exhaustion on ESP8266
    
    HTTPClient http;
    http.setTimeout(8000);
    
    String url = xor_decrypt(URL, URL_LEN, 0x5A);
    String token = xor_decrypt(TOKEN, TOKEN_LEN, 0x5A);
    
    if (http.begin(client, url)) {
        http.addHeader("Content-Type", "application/json");
        http.addHeader("X-WeatherStation-Ping-Token", token);
        
        JsonDocument doc;
        doc["uuid"] = get_ping_device_id();
        doc["event"] = event_type;
        doc["version"] = FIRMWARE_VERSION;
        doc["uptime"] = millis() / 1000;
        
        // Auto-diagnostics stored in the D1 table's crash_dump column
        String diagnostics = "Free Heap: " + String(ESP.getFreeHeap()) + " B";
        if (event_type == "boot" || event_type == "reboot") {
            diagnostics = "Reset Reason: " + ESP.getResetReason() + " | Info: " + ESP.getResetInfo() + " | " + diagnostics;
        }
        doc["crash_dump"] = diagnostics;
        
        String body;
        serializeJson(doc, body);
        
        http.POST(body);
        http.end();
    }
}

EspSaveCrash SaveCrash;
bool opt_in_crash_dump = true;

#include <EEPROM.h>

String get_safe_crash_dump() {
    EEPROM.begin(SaveCrash.offset() + SaveCrash.size());
    byte crashCounter = EEPROM.read(SaveCrash.offset() + SAVE_CRASH_COUNTER);
    if (crashCounter == 0 || crashCounter > 10) {
        EEPROM.end();
        return "";
    }
    
    String dump = "Crash Counter: " + String(crashCounter) + "\n";
    int16_t readFrom = SaveCrash.offset() + SAVE_CRASH_DATA_SETS;
    
    byte k = crashCounter - 1; // latest crash
    int16_t crashOffset = readFrom;
    for (byte i = 0; i < k; i++) {
        uint32_t stackStart, stackEnd;
        EEPROM.get(crashOffset + SAVE_CRASH_STACK_START, stackStart);
        EEPROM.get(crashOffset + SAVE_CRASH_STACK_END, stackEnd);
        crashOffset += SAVE_CRASH_STACK_TRACE + (stackEnd - stackStart);
    }
    
    uint32_t crashTime;
    byte reason, exception;
    uint32_t epc1, epc2, epc3, excvaddr, depc;
    uint32_t stackStart, stackEnd;
    
    EEPROM.get(crashOffset + SAVE_CRASH_CRASH_TIME, crashTime);
    reason = EEPROM.read(crashOffset + SAVE_CRASH_RESTART_REASON);
    exception = EEPROM.read(crashOffset + SAVE_CRASH_EXCEPTION_CAUSE);
    EEPROM.get(crashOffset + SAVE_CRASH_EPC1, epc1);
    EEPROM.get(crashOffset + SAVE_CRASH_EPC2, epc2);
    EEPROM.get(crashOffset + SAVE_CRASH_EPC3, epc3);
    EEPROM.get(crashOffset + SAVE_CRASH_EXCVADDR, excvaddr);
    EEPROM.get(crashOffset + SAVE_CRASH_DEPC, depc);
    EEPROM.get(crashOffset + SAVE_CRASH_STACK_START, stackStart);
    EEPROM.get(crashOffset + SAVE_CRASH_STACK_END, stackEnd);
    
    char buf[256];
    snprintf(buf, sizeof(buf), 
             "Latest Crash Info:\n"
             "Time: %u ms\n"
             "Reason: %d\n"
             "Exception Cause: %d\n"
             "epc1: 0x%08x\n"
             "epc2: 0x%08x\n"
             "epc3: 0x%08x\n"
             "excvaddr: 0x%08x\n"
             "depc: 0x%08x\n"
             "Stack: 0x%08x - 0x%08x\n",
             crashTime, reason, exception, epc1, epc2, epc3, excvaddr, depc, stackStart, stackEnd);
    
    dump += String(buf);
    dump += "Stack Trace (first 64 bytes): ";
    
    int16_t currentAddress = crashOffset + SAVE_CRASH_STACK_TRACE;
    int16_t stackLength = stackEnd - stackStart;
    int16_t bytesToRead = (stackLength > 64) ? 64 : stackLength;
    if (bytesToRead < 0) bytesToRead = 0;
    
    for (int16_t i = 0; i < bytesToRead; i++) {
        byte val = EEPROM.read(currentAddress + i);
        char hex[3];
        snprintf(hex, sizeof(hex), "%02x", val);
        dump += hex;
    }
    
    EEPROM.end();
    return dump;
}

void check_and_send_crash_dump() {
    int crash_count = SaveCrash.count();
    if (crash_count > 10 || crash_count < 0) {
        SaveCrash.clear();
        return;
    }
    
    if (crash_count > 0) {
        if (!opt_in_crash_dump) {
            SaveCrash.clear();
            return;
        }
        
        String dump = get_safe_crash_dump();
        if (dump == "") {
            SaveCrash.clear();
            return;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            WiFiClientSecure client;
            client.setInsecure();
            client.setBufferSizes(1024, 1024);
            
            HTTPClient http;
            http.setTimeout(10000);
            
            String url = xor_decrypt(URL, URL_LEN, 0x5A);
            String token = xor_decrypt(TOKEN, TOKEN_LEN, 0x5A);
            
            if (http.begin(client, url)) {
                http.addHeader("Content-Type", "application/json");
                http.addHeader("X-WeatherStation-Ping-Token", token);
                
                JsonDocument doc;
                doc["uuid"] = get_ping_device_id();
                doc["event"] = "crash";
                doc["version"] = FIRMWARE_VERSION;
                doc["uptime"] = millis() / 1000;
                doc["crash_dump"] = dump;
                
                String body;
                serializeJson(doc, body);
                
                int code = http.POST(body);
                if (code == 200) {
                    SaveCrash.clear();
                }
                http.end();
            }
        }
    }
}
Ticker led_ticker;
const int LED_PIN = 2; // GPIO 2 is the onboard blue LED on ESP-12E (active-LOW)
bool led_ticker_active = false;

void IRAM_ATTR tick_led() {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

void start_led_fast_blink() {
    if (!led_ticker_active) {
        led_ticker.attach(0.1, tick_led);
        led_ticker_active = true;
    }
}

void set_led_connected() {
    if (led_ticker_active) {
        led_ticker.detach();
        led_ticker_active = false;
    }
    digitalWrite(LED_PIN, LOW); // solid ON (active-LOW)
}

void led_blink_mqtt() {
    if (led_ticker_active) return; // Don't interrupt if already flashing fast (disconnected)
    
    // Double blink sequence: OFF -> ON -> OFF -> ON
    digitalWrite(LED_PIN, HIGH); // OFF
    delay(80);
    digitalWrite(LED_PIN, LOW);  // ON (1st blink)
    delay(100);
    digitalWrite(LED_PIN, HIGH); // OFF
    delay(100);
    digitalWrite(LED_PIN, LOW);  // ON (2nd blink)
    delay(100);
    digitalWrite(LED_PIN, LOW);  // Keep solid ON (connected state)
}
#include "web_server.h"
#include "mqtt_manager.h"
#include <WiFiManager.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <ScioSense_ENS16x.h>

static WiFiEventHandler onStationConnectedHandler;
static WiFiEventHandler onStationDisconnectedHandler;
static WiFiEventHandler onStationGotIPHandler;

const char* get_wifi_disconnection_reason_str(uint8_t reason) {
    switch(reason) {
        case 1: return "UNSPECIFIED";
        case 2: return "AUTH_EXPIRE";
        case 3: return "AUTH_LEAVE";
        case 4: return "ASSOC_EXPIRE";
        case 5: return "ASSOC_TOOMANY";
        case 6: return "NOT_AUTHED";
        case 7: return "NOT_ASSOCED";
        case 8: return "ASSOC_LEAVE";
        case 9: return "ASSOC_NOT_AUTHED";
        case 13: return "IE_INVALID";
        case 14: return "MIC_FAILURE";
        case 15: return "4WAY_HANDSHAKE_TIMEOUT (Check pass / WPA3 / PMF mode)";
        case 16: return "GROUP_KEY_UPDATE_TIMEOUT";
        case 19: return "PAIRWISE_CIPHER_INVALID (WPA3 / PMF incompatible mode)";
        case 24: return "CIPHER_SUITE_REJECTED";
        case 200: return "BEACON_TIMEOUT (Signal weak or AP on 5GHz/different channel)";
        case 201: return "NO_AP_FOUND (SSID not found - ESP8266 only supports 2.4GHz!)";
        case 202: return "AUTH_FAIL";
        case 203: return "ASSOC_FAIL";
        case 204: return "HANDSHAKE_TIMEOUT";
        default: return "UNKNOWN_REASON";
    }
}

ADC_MODE(ADC_VCC);

// --- Global variables definition ---
volatile uint32_t total_bucket_tips = 0;
float total_rain_mm = 0.0;
float rolling_rain_hour = 0.0;
float rolling_rain_day = 0.0;
bool is_raining = false;

uint16_t rain_history[HISTORY_MINUTES] = {0};
int current_minute_index = 0;
unsigned long last_minute_update = 0;

String hostname = "WeatherStation";
bool use_dhcp = false;
bool use_imperial = false;
bool ui_compact = false;
String wifi_ssid = "";
String wifi_pass = "";
String wifi_ip = "";
String wifi_gw = "";
String wifi_nm = "";
float rain_calibration = 0.6314f; // 6ml volume / 95.03 cm^2 area = 0.6314 mm per tip
uint32_t rain_debounce_ms = 300;   // 300 ms debounce
int rain_sensor_pin = 14;         // Default pin GPIO14
int i2c_sda_pin = 4;
int i2c_scl_pin = 5;
int i2c_clock_khz = 100;
int altitude_m = 0;
float temp_offset = 0.0f;
float hum_offset = 0.0f;
float press_offset = 0.0f;
bool debug_logs_enabled = false;
int mqtt_publish_interval_s = 15;
int sensor_read_interval_s = 5;
int mqtt_decimals = 1;
String dns_primary = "";
String dns_secondary = "";
String ntp_server = "pool.ntp.org";

Preferences prefs;

void update_rain_rolling_totals() {
    uint32_t hour_tips = 0;
    uint32_t day_tips = 0;
    uint32_t five_min_tips = 0;

    for (int i = 0; i < 60; i++) {
        int idx = (current_minute_index - i + HISTORY_MINUTES) % HISTORY_MINUTES;
        hour_tips += rain_history[idx];
    }

    for (int i = 0; i < HISTORY_MINUTES; i++) {
        day_tips += rain_history[i];
    }

    for (int i = 0; i < 5; i++) {
        int idx = (current_minute_index - i + HISTORY_MINUTES) % HISTORY_MINUTES;
        five_min_tips += rain_history[idx];
    }

    rolling_rain_hour = hour_tips * rain_calibration;
    rolling_rain_day = day_tips * rain_calibration;
    is_raining = (five_min_tips > 0);
}

// --- Environmental sensors definition ---
bool has_aht20 = false;
bool has_bmp280 = false;
float temperature_c = 0.0f;
float humidity_pct = 0.0f;
float pressure_hpa = 0.0f;

// --- ENS160 Air Quality sensor definition ---
bool has_ens160 = false;
uint16_t ens160_tvoc = 0;
uint16_t ens160_eco2 = 0;
uint8_t ens160_aqi = 0;

Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;
ENS160 ens160;

// --- Anemometer definition ---
float wind_speed_kmh = 0.0f;
float wind_gust_kmh = 0.0f;
int wind_sensor_pin = 12;
float wind_calibration = 5.42867f;
int wind_radius_mm = 80;
int wind_magnets = 1;
float wind_factor = 3.0f;
uint32_t wind_debounce_ms = 15;

// --- AS5600 definition ---
bool has_as5600 = false;
float wind_dir_deg = 0.0f;
int wind_dir_offset = 0;

// --- BH1750 definition ---
bool has_bh1750 = false;
float lux = 0.0f;
float lux_cal_factor = 1.0f;
float unfiltered_lux_ref = 0.0f;

volatile uint32_t wind_pulse_count = 0;
volatile unsigned long last_wind_interrupt_time = 0;

// --- Moving Average configuration ---
int wind_speed_avg_samples = 5;   // Number of speed samples for rolling average (configurable)
int wind_dir_avg_samples   = 5;   // Number of direction samples for rolling average (configurable)
int wind_speed_interval_s  = 2;   // Wind speed sampling interval in seconds (independent of I2C sensors)

// --- Moving Average ring buffers ---
float wind_speed_buf[WIND_AVG_MAX_SAMPLES] = {0};
float wind_dir_buf[WIND_AVG_MAX_SAMPLES]   = {0};
int   wind_speed_buf_idx = 0;
int   wind_dir_buf_idx   = 0;
int   wind_speed_buf_count = 0;   // Filled slots (up to wind_speed_avg_samples)
int   wind_dir_buf_count   = 0;

void IRAM_ATTR wind_sensor_isr() {
    unsigned long now = millis();
    if (now - last_wind_interrupt_time >= wind_debounce_ms) {
        wind_pulse_count++;
        last_wind_interrupt_time = now;
    }
}

void reset_wind_gust() {
    wind_gust_kmh = 0.0f;
    prefs.begin("weather", false);
    prefs.putFloat("w_gust", 0.0f);
    prefs.end();
    app_log("Wind gust counter cleared.");
}

// Global tracking variables
uint32_t last_processed_tips = 0;
static volatile unsigned long last_tip_time = 0;

// Interrupt Service Routine for tipping bucket
void IRAM_ATTR rain_sensor_isr() {
    unsigned long now = millis();
    if (now - last_tip_time >= rain_debounce_ms) {
        total_bucket_tips++;
        last_tip_time = now;
    }
}

// Function to attach the interrupt
void attach_sensor_interrupt() {
    pinMode(rain_sensor_pin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(rain_sensor_pin), rain_sensor_isr, FALLING);
    app_log("Rain gauge interrupt configured on GPIO %d", rain_sensor_pin);
}

// Recompute rolling rain rates based on history array
void recalculate_rolling_rain() {
    uint32_t hour_tips = 0;
    uint32_t day_tips = 0;
    uint32_t five_min_tips = 0;

    for (int i = 0; i < HISTORY_MINUTES; i++) {
        uint16_t tips = rain_history[i];
        day_tips += tips;

        // Calculate distance from current index
        int dist = (current_minute_index - i + HISTORY_MINUTES) % HISTORY_MINUTES;
        if (dist < 60) {
            hour_tips += tips;
        }
        if (dist < 5) {
            five_min_tips += tips;
        }
    }

    rolling_rain_hour = hour_tips * rain_calibration;
    rolling_rain_day = day_tips * rain_calibration;
    is_raining = (five_min_tips > 0);
}

void load_settings() {
    prefs.begin("weather", false);
    
    hostname          = prefs.getString("hostname", "WeatherStation");
    rain_sensor_pin   = prefs.getInt("pin", 14);
    rain_calibration  = prefs.getFloat("cal", 0.6314f);
    rain_debounce_ms  = prefs.getInt("debounce", 300);
    use_dhcp          = prefs.getBool("dhcp", false);
    use_imperial      = prefs.getBool("use_imp", false);
    ui_compact        = prefs.getBool("ui_comp", false);
    wifi_ssid         = prefs.getString("wifi_ssid", "");
    wifi_pass         = prefs.getString("wifi_pass", "");
    wifi_ip           = prefs.getString("ip", "");
    wifi_gw           = prefs.getString("gw", "");
    wifi_nm           = prefs.getString("nm", "");
    dns_primary       = prefs.getString("dns_p", "");
    dns_secondary     = prefs.getString("dns_s", "8.8.8.8");
    ntp_server        = prefs.getString("ntp", "pool.ntp.org");

    // I2C / Environmental sensors
    i2c_sda_pin           = prefs.getInt("sda", 4);
    i2c_scl_pin           = prefs.getInt("scl", 5);
    i2c_clock_khz         = prefs.getInt("i2c_clk", 100);
    altitude_m            = prefs.getInt("alt", 0);
    temp_offset           = prefs.getFloat("t_offset", 0.0f);
    hum_offset            = prefs.getFloat("h_offset", 0.0f);
    press_offset          = prefs.getFloat("p_offset", 0.0f);
    sensor_read_interval_s = prefs.getInt("sens_int", 5);

    // Wind / Anemometer sensor
    wind_sensor_pin   = prefs.getInt("w_pin", 12);
    wind_radius_mm    = prefs.getInt("w_rad", 80);
    wind_magnets      = prefs.getInt("w_mag", 1);
    wind_factor       = prefs.getFloat("w_fac", 3.0f);
    wind_calibration  = prefs.getFloat("w_cal", 5.42867f);
    wind_debounce_ms  = prefs.getInt("w_deb", 15);
    wind_dir_offset   = prefs.getInt("w_dir_off", 0);
    wind_speed_avg_samples = prefs.getInt("w_spd_avg", 5);
    wind_dir_avg_samples   = prefs.getInt("w_dir_avg", 5);
    wind_speed_interval_s  = prefs.getInt("w_spd_int", 2);

    // Lux sensor calibration
    lux_cal_factor    = prefs.getFloat("lux_cal", 1.0f);

    // MQTT
    mqtt_publish_interval_s = prefs.getInt("mqtt_int", 15);
    mqtt_decimals           = prefs.getInt("mqtt_dec", 1);

    // Load saved tips count from flash
    total_bucket_tips  = prefs.getLong("tips", 0);
    last_processed_tips = total_bucket_tips;
    total_rain_mm      = total_bucket_tips * rain_calibration;

    WiFi.hostname(hostname);
    
    opt_in_crash_dump  = prefs.getBool("crash_opt", true);
    is_reboot_pending  = prefs.getBool("reboot_pending", false);
    if (is_reboot_pending) {
        prefs.putBool("reboot_pending", false);
    }
    prefs.end();
}

uint16_t read_as5600_raw_angle() {
    Wire.beginTransmission(0x36);
    Wire.write(0x0C); // RAW ANGLE register
    if (Wire.endTransmission() != 0) {
        return 0xFFFF; // Error
    }
    Wire.requestFrom(0x36, 2);
    if (Wire.available() >= 2) {
        uint16_t raw = Wire.read();
        raw <<= 8;
        raw |= Wire.read();
        return raw & 0x0FFF; // 12-bit
    }
    return 0xFFFF;
}

void read_wind_direction() {
    if (has_as5600) {
        uint16_t raw = read_as5600_raw_angle();
        if (raw != 0xFFFF) {
            float deg = (float)raw * 360.0f / 4096.0f;
            float calibrated_deg = deg - wind_dir_offset;
            if (calibrated_deg < 0.0f)   calibrated_deg += 360.0f;
            if (calibrated_deg >= 360.0f) calibrated_deg -= 360.0f;

            // Push into ring buffer
            int n = max(1, min(wind_dir_avg_samples, WIND_AVG_MAX_SAMPLES));
            wind_dir_buf[wind_dir_buf_idx] = calibrated_deg;
            wind_dir_buf_idx = (wind_dir_buf_idx + 1) % n;
            if (wind_dir_buf_count < n) wind_dir_buf_count++;

            // Circular mean to handle 0°/360° wrap-around correctly
            float sin_sum = 0.0f, cos_sum = 0.0f;
            for (int i = 0; i < wind_dir_buf_count; i++) {
                float rad = wind_dir_buf[i] * (3.14159265f / 180.0f);
                sin_sum += sinf(rad);
                cos_sum += cosf(rad);
            }
            float mean_deg = atan2f(sin_sum, cos_sum) * (180.0f / 3.14159265f);
            if (mean_deg < 0.0f) mean_deg += 360.0f;
            wind_dir_deg = mean_deg;

            if (debug_logs_enabled) {
                app_log("[I2C Debug] [AS5600] Raw: %u, Instant: %.1f deg, Avg(%d): %.1f deg",
                        raw, calibrated_deg, wind_dir_buf_count, wind_dir_deg);
            }
        } else {
            if (debug_logs_enabled) {
                app_log("[I2C Debug] [AS5600] Discarded - Read failed");
            }
        }
    }
}
void read_lux_sensor() {
    if (has_bh1750) {
        Wire.requestFrom(0x23, 2);
        if (Wire.available() >= 2) {
            uint16_t raw = Wire.read();
            raw <<= 8;
            raw |= Wire.read();
            float raw_lux = (float)raw / 1.2f;
            if (lux_cal_factor > 0.001f) {
                lux = raw_lux / lux_cal_factor;
            } else {
                lux = raw_lux;
            }
            if (debug_logs_enabled) {
                app_log("[I2C Debug] [BH1750] Accepted read - Raw: %u, Lux: %s lx", raw, String(lux, 1).c_str());
            }
        } else {
            if (debug_logs_enabled) {
                app_log("[I2C Debug] [BH1750] Discarded - Read failed");
            }
        }
    }
}
void read_environmental_sensors() {
    // Check AHT20 status and reset if reading fails or is invalid
    if (has_aht20) {
        sensors_event_t humidity = {0};
        sensors_event_t temp = {0};
        if (aht.getEvent(&humidity, &temp)) {
            if (temp.temperature < -45.0f || temp.temperature > 80.0f || 
                humidity.relative_humidity < 0.0f || humidity.relative_humidity > 100.0f) {
                app_log("[AHT20] Invalid readings (T: %s C, H: %s %%). Resetting sensor status...", 
                        String(temp.temperature, 1).c_str(), String(humidity.relative_humidity, 1).c_str());
                if (debug_logs_enabled) {
                    app_log("[I2C Debug] [AHT20] Discarded invalid read - Temp: %s C, Hum: %s %%", 
                            String(temp.temperature, 1).c_str(), String(humidity.relative_humidity, 1).c_str());
                }
                has_aht20 = false;
            } else {
                if (debug_logs_enabled) {
                    app_log("[I2C Debug] [AHT20] Accepted read - Temp: %s C, Hum: %s %%", 
                            String(temp.temperature, 1).c_str(), String(humidity.relative_humidity, 1).c_str());
                }
            }
        } else {
            app_log("[AHT20] Read event failed. Resetting sensor status...");
            if (debug_logs_enabled) {
                app_log("[I2C Debug] [AHT20] Discarded - Read event failed");
            }
            has_aht20 = false;
        }
    }

    // Dynamic initialization of AHT20 if not present
    if (!has_aht20) {
        if (aht.begin()) {
            has_aht20 = true;
            app_log("AHT20 sensor dynamically initialized successfully.");
        }
    }

    // Check BMP280 status and reset if reading is garbage or invalid
    if (has_bmp280) {
        float temp = bmp.readTemperature();
        float press = bmp.readPressure() / 100.0f; // Pa to hPa
        if (temp > 80.0f || temp < -40.0f || press < 800.0f || press > 1200.0f) {
            app_log("[BMP280] Invalid readings (T: %s C, P: %s hPa). Resetting sensor status...", 
                    String(temp, 1).c_str(), String(press, 1).c_str());
            if (debug_logs_enabled) {
                app_log("[I2C Debug] [BMP280] Discarded invalid read - Temp: %s C, Press: %s hPa", 
                        String(temp, 1).c_str(), String(press, 1).c_str());
            }
            has_bmp280 = false;
        } else {
            if (debug_logs_enabled) {
                app_log("[I2C Debug] [BMP280] Accepted read - Temp: %s C, Press: %s hPa", 
                        String(temp, 1).c_str(), String(press, 1).c_str());
            }
        }
    }

    // Dynamic initialization of BMP280 if not present
    if (!has_bmp280) {
        if (bmp.begin(0x76) || bmp.begin(0x77)) {
            has_bmp280 = true;
            app_log("BMP280 sensor dynamically initialized successfully.");
        }
    }

    float raw_temp = 0.0f;
    float raw_hum = 0.0f;

    // Read AHT21 (for humidity, and fallback temperature)
    if (has_aht20) {
        sensors_event_t humidity = {0};
        sensors_event_t temp = {0};
        if (aht.getEvent(&humidity, &temp)) {
            raw_hum = humidity.relative_humidity;
            if (!has_bmp280) {
                raw_temp = temp.temperature;
            }
        }
    }

    // Read BMP280 temperature (takes priority)
    if (has_bmp280) {
        raw_temp = bmp.readTemperature();
    }

    // Apply temperature offset
    if (has_aht20 || has_bmp280) {
        temperature_c = raw_temp + temp_offset;
    } else {
        temperature_c = 0.0f;
    }

    // Apply humidity offset
    if (has_aht20) {
        humidity_pct = raw_hum + hum_offset;
        if (humidity_pct < 0.0f) humidity_pct = 0.0f;
        if (humidity_pct > 100.0f) humidity_pct = 100.0f;
    } else {
        humidity_pct = 0.0f;
    }

    if (has_bmp280) {
        float abs_pressure = bmp.readPressure() / 100.0F; // Pa to hPa
        if (altitude_m > 0) {
            // NOAA sea level pressure formula
            pressure_hpa = abs_pressure * pow(1.0f - (0.0065f * altitude_m) / (temperature_c + 0.0065f * altitude_m + 273.15f), -5.257f);
        } else {
            pressure_hpa = abs_pressure;
        }
        pressure_hpa += press_offset;
    } else {
        pressure_hpa = 0.0f;
    }

    // Dynamic initialization of ENS160 if it wasn't ready at boot
    if (!has_ens160) {
        ens160.begin(&Wire, 0x53);
        if (ens160.init()) {
            has_ens160 = true;
            ens160.startStandardMeasure();
            app_log("ENS160 gas sensor dynamically initialized at 0x53.");
        } else {
            ens160.begin(&Wire, 0x52);
            if (ens160.init()) {
                has_ens160 = true;
                ens160.startStandardMeasure();
                app_log("ENS160 gas sensor dynamically initialized at 0x52.");
            }
        }
    }

    // Read ENS160 if present
    if (has_ens160) {
        // Write compensation if we have a valid temp/humidity reading
        if (has_aht20 || has_bmp280) {
            uint16_t t_comp = Ens16x_CalcTempInFromCelsius(temperature_c);
            uint16_t h_comp = Ens16x_CalcRhIn(has_aht20 ? humidity_pct : 50.0f);
            ens160.writeCompensation(t_comp, h_comp);
        }
        
        ens160.update();
        ens160_tvoc = ens160.getTvoc();
        ens160_eco2 = ens160.getEco2();
        ens160_aqi = ens160.getAirQualityIndex_UBA();
        if (debug_logs_enabled) {
            app_log("[I2C Debug] [ENS160] Accepted read - TVOC: %u ppb, eCO2: %u ppm, AQI: %d", 
                    ens160_tvoc, ens160_eco2, ens160_aqi);
        }
    } else {
        ens160_tvoc = 0;
        ens160_eco2 = 0;
        ens160_aqi = 0;
    }

    // Read AS5600 wind direction
    read_wind_direction();

    // Read BH1750 ambient light
    read_lux_sensor();
}

void setup() {
    

    // Configure onboard blue LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // Start OFF
    start_led_fast_blink(); // Fast flash indicating boot/connecting

    Serial.begin(115200);
    delay(500);
    
    app_log("=== SMART WEATHER STATION BOOT ===");
    app_log("[System] Reset Reason: %s", ESP.getResetReason().c_str());
    app_log("[System] Reset Info: %s", ESP.getResetInfo().c_str());
    
    load_settings();
    
    // Initialize I2C and environmental sensors
    Wire.begin(i2c_sda_pin, i2c_scl_pin);
    Wire.setClock(i2c_clock_khz * 1000);
    delay(150); // Give I2C devices time to power on and stabilize
    
    if (aht.begin()) {
        has_aht20 = true;
        app_log("AHT20 sensor initialized successfully.");
    } else {
        app_log("AHT20 sensor not found!");
    }

    if (bmp.begin(0x76) || bmp.begin(0x77)) {
        has_bmp280 = true;
        app_log("BMP280 sensor initialized successfully.");
    } else {
        app_log("BMP280 sensor not found!");
    }

    // Initialize ENS160 gas sensor (giving it a short delay to boot)
    delay(200);
    ens160.begin(&Wire, 0x53);
    if (ens160.init()) {
        has_ens160 = true;
        ens160.startStandardMeasure();
        app_log("ENS160 gas sensor initialized successfully at 0x53.");
    } else {
        ens160.begin(&Wire, 0x52);
        if (ens160.init()) {
            has_ens160 = true;
            ens160.startStandardMeasure();
            app_log("ENS160 gas sensor initialized successfully at 0x52.");
        } else {
            app_log("ENS160 gas sensor not found at boot (will retry in loop).");
        }
    }

    // Initialize AS5600 wind vane sensor
    Wire.beginTransmission(0x36);
    if (Wire.endTransmission() == 0) {
        has_as5600 = true;
        app_log("AS5600 wind direction sensor initialized successfully.");
    } else {
        app_log("AS5600 sensor not found!");
    }

    // Initialize BH1750 lux sensor
    Wire.beginTransmission(0x23);
    if (Wire.endTransmission() == 0) {
        has_bh1750 = true;
        // Start continuous H-resolution mode (1 lx resolution)
        Wire.beginTransmission(0x23);
        Wire.write(0x10);
        Wire.endTransmission();
        app_log("BH1750 lux sensor initialized successfully.");
        delay(200); // Wait for the first measurement to complete
    } else {
        app_log("BH1750 sensor not found!");
    }

    read_environmental_sensors();
    
    // Setup rain sensor hardware interrupt
    attach_sensor_interrupt();

    // Setup wind sensor hardware interrupt
    pinMode(wind_sensor_pin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(wind_sensor_pin), wind_sensor_isr, FALLING);
    app_log("Wind sensor interrupt configured on GPIO %d", wind_sensor_pin);

    // Setup FLASH button (GPIO 0) for 10-second hold factory reset
    pinMode(0, INPUT_PULLUP);

    // Configure WiFi radio settings for maximum reliability during association
    WiFi.persistent(true);
    WiFi.setSleepMode(WIFI_NONE_SLEEP); // Disable modem sleep to avoid dropped 4-way handshake packets

    // Register WiFi event handlers for precise SDK diagnostic logs
    onStationConnectedHandler = WiFi.onStationModeConnected([](const WiFiEventStationModeConnected& evt) {
        app_log("[WiFi EVENT] Associated with SSID: '%s' | BSSID: %02X:%02X:%02X:%02X:%02X:%02X | Channel: %d",
                evt.ssid.c_str(),
                evt.bssid[0], evt.bssid[1], evt.bssid[2],
                evt.bssid[3], evt.bssid[4], evt.bssid[5],
                evt.channel);
    });

    onStationDisconnectedHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& evt) {
        app_log("[WiFi EVENT DIAGNOSTIC] Disconnected from SSID: '%s' | Reason Code: %d (%s)",
                evt.ssid.c_str(),
                (int)evt.reason,
                get_wifi_disconnection_reason_str(evt.reason));
    });

    onStationGotIPHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& evt) {
        app_log("[WiFi EVENT] Got IP: %s | Subnet: %s | Gateway: %s",
                evt.ip.toString().c_str(),
                evt.mask.toString().c_str(),
                evt.gw.toString().c_str());
    });

    // Ensure hostname is set in SDK before station mode initialization
    wifi_station_set_hostname(hostname.c_str());
    WiFi.hostname(hostname);
    WiFi.persistent(true);
    WiFi.mode(WIFI_STA);
    WiFi.setSleepMode(WIFI_NONE_SLEEP); // Essential for FRITZ!Box 4-way handshake

    bool direct_connected = false;

    // Attempt direct connection only if saved wifi_ssid exists
    if (wifi_ssid.length() > 0) {
        IPAddress target_ip, target_gw, target_nm, target_dns;
        bool has_static_config = target_ip.fromString(wifi_ip) && 
                                 target_gw.fromString(wifi_gw) && 
                                 target_nm.fromString(wifi_nm);
        target_dns.fromString(dns_primary);

        if (use_dhcp || !has_static_config) {
            app_log("[WiFi] Mode: DHCP (Automatic IP)");
            WiFi.disconnect(true);
            delay(100);
            wifi_station_dhcpc_stop();
            wifi_station_dhcpc_start();
        } else {
            app_log("[WiFi] Mode: Static IP (%s)", wifi_ip.c_str());
            WiFi.config(target_ip, target_gw, target_nm, target_dns);
        }

        WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
        app_log("[WiFi] Connecting to '%s'...", wifi_ssid.c_str());

        unsigned long wifi_start = millis();
        while (millis() - wifi_start < 15000) {
            if (WiFi.status() == WL_CONNECTED && WiFi.localIP() != IPAddress(0, 0, 0, 0)) {
                direct_connected = true;
                break;
            }
            delay(250);
        }
    }

    if (direct_connected) {
        set_led_connected();
        app_log("[WiFi SUCCESS] Connected to '%s'! IP: %s | Channel: %d | RSSI: %d dBm",
                WiFi.SSID().c_str(),
                WiFi.localIP().toString().c_str(),
                WiFi.channel(),
                WiFi.RSSI());
    } else {
        app_log("[WiFi] Starting WiFi autoconnect portal '%s_Setup'...", hostname.c_str());
        
        WiFiManager wm;
        wm.setDebugOutput(true);
        wm.setCleanConnect(true);
        wm.setWiFiAutoReconnect(true);
        wm.setHostname(hostname.c_str());
        wm.setConfigPortalTimeout(180);

        WiFiManagerParameter custom_header("<h3>Static IP parameter (Required if you have issue whit DHCP)</h3>");
        // inputmode="decimal" triggers numeric keypad with decimal point on iOS/Android
        WiFiManagerParameter custom_ip("ip", "Static IP", wifi_ip.c_str(), 16, "inputmode=\"decimal\" pattern=\"[0-9.]*\" placeholder=\"es. 192.168.1.100\"");
        WiFiManagerParameter custom_gw("gw", "Gateway (Router)", wifi_gw.c_str(), 16, "inputmode=\"decimal\" pattern=\"[0-9.]*\" placeholder=\"es. 192.168.1.1\"");
        WiFiManagerParameter custom_nm("nm", "Subnet Mask", wifi_nm.c_str(), 16, "inputmode=\"decimal\" pattern=\"[0-9.]*\" placeholder=\"es. 255.255.255.0\"");
        WiFiManagerParameter custom_dns("dns", "DNS", dns_primary.c_str(), 16, "inputmode=\"decimal\" pattern=\"[0-9.]*\" placeholder=\"es. 8.8.8.8\"");

        wm.addParameter(&custom_header);
        wm.addParameter(&custom_ip);
        wm.addParameter(&custom_gw);
        wm.addParameter(&custom_nm);
        wm.addParameter(&custom_dns);

        wm.setAPCallback([](WiFiManager *myWiFiManager) {
            app_log("[WiFi] Config portal activated! AP SSID: '%s', AP IP: %s",
                    myWiFiManager->getConfigPortalSSID().c_str(),
                    WiFi.softAPIP().toString().c_str());
        });

        wm.setSaveParamsCallback([&custom_ip, &custom_gw, &custom_nm, &custom_dns]() {
            app_log("[WiFi] Network parameters saved from WiFiManager portal!");
            prefs.begin("weather", false);
            if (strlen(custom_ip.getValue()) > 0 && strlen(custom_gw.getValue()) > 0) {
                prefs.putBool("dhcp", false);
                prefs.putString("ip", custom_ip.getValue());
                prefs.putString("gw", custom_gw.getValue());
                wifi_ip = custom_ip.getValue();
                wifi_gw = custom_gw.getValue();
            }
            if (strlen(custom_nm.getValue()) > 0) {
                prefs.putString("nm", custom_nm.getValue());
                wifi_nm = custom_nm.getValue();
            }
            if (strlen(custom_dns.getValue()) > 0) {
                prefs.putString("dns_p", custom_dns.getValue());
                dns_primary = custom_dns.getValue();
            }
            prefs.end();
        });

        String ap_name = hostname + "_Setup";
        if (!wm.autoConnect(ap_name.c_str())) {
            app_log("[WiFi ERROR] WiFiManager portal timed out. Restarting ESP in 5 seconds...");
            delay(5000);
            ESP.restart();
        }

        // Save newly connected WiFi credentials
        prefs.begin("weather", false);
        prefs.putString("wifi_ssid", WiFi.SSID());
        prefs.putString("wifi_pass", WiFi.psk());
        prefs.end();

        set_led_connected();
        app_log("[WiFi SUCCESS] Connected via WiFiManager! Restarting cleanly to finalize network routing...");
        delay(1000);
        ESP.restart();
    }
    

    configTime(0, 0, ntp_server.c_str(), "time.nist.gov");
    app_log("NTP time synchronization initialized (GMT-0).");

    // Initialize systems
    setup_web_server();
    setup_mqtt();

    last_minute_update = millis();
    app_log("Initialization completed successfully!");
}

void loop() {
    // Check 10-second FLASH button press for Factory Reset (Flash Erase & Restart)
    static unsigned long flash_button_press_start = 0;
    if (digitalRead(0) == LOW) { // FLASH button pressed (GPIO 0 connected to GND)
        if (flash_button_press_start == 0) {
            flash_button_press_start = millis();
            app_log("[Button] FLASH button pressed. Hold for 10 seconds to factory reset...");
        } else if (millis() - flash_button_press_start >= 10000) {
            app_log("[FACTORY RESET] FLASH button held for 10 seconds! Erasing flash preferences and restarting...");
            
            // Rapid visual LED feedback (Fast Blink for 2 seconds)
            for (int i = 0; i < 20; i++) {
                digitalWrite(LED_BUILTIN, LOW); // ON
                delay(50);
                digitalWrite(LED_BUILTIN, HIGH); // OFF
                delay(50);
            }

            // Clear NVS preferences (erases WiFi SSID/Pass, IP config, all saved settings)
            prefs.begin("weather", false);
            prefs.clear();
            prefs.end();

            WiFi.disconnect(true);
            delay(500);
            ESP.restart();
        }
    } else {
        flash_button_press_start = 0; // Reset timer when button is released
    }

    // Check and send crash dump if present (staggered to prevent heap exhaustion)
    static bool crash_checked = false;
    static bool ping_boot_sent = false;
    static unsigned long ntp_sync_time = 0;
    
    if (WiFi.status() == WL_CONNECTED && time(nullptr) > 1700000000UL) {
        if (ntp_sync_time == 0) {
            ntp_sync_time = millis();
        }
        
        // 1. Check/send crash dump 5 seconds after NTP sync
        if (!crash_checked && (millis() - ntp_sync_time > 5000)) {
            crash_checked = true;
            check_and_send_crash_dump();
        }
        
        // 2. Send boot/reboot ping 15 seconds after NTP sync
        if (!ping_boot_sent && (millis() - ntp_sync_time > 15000)) {
            ping_boot_sent = true;
            send_cloud_ping(is_reboot_pending ? "reboot" : "boot");
        }
    }

    // Daily alive ping
    static unsigned long last_alive_sent = 0;
    if (WiFi.status() == WL_CONNECTED) {
        if (last_alive_sent == 0) {
            last_alive_sent = millis();
        } else if (millis() - last_alive_sent >= 86400000UL) {
            last_alive_sent = millis();
            send_cloud_ping("alive");
        }
    }

    // Manage LED state based on Wi-Fi connection status
    static bool last_wifi_connected = false;
    bool current_wifi_connected = (WiFi.status() == WL_CONNECTED);
    if (current_wifi_connected != last_wifi_connected) {
        last_wifi_connected = current_wifi_connected;
        if (current_wifi_connected) {
            set_led_connected();
        } else {
            start_led_fast_blink();
        }
    }

    // Process client requests
    handle_web_server();
    
    // Handle MQTT loop and reconnects
    handle_mqtt();

    // 1. Check if the interrupt detected new tips
    uint32_t current_tips = total_bucket_tips;
    if (current_tips > last_processed_tips) {
        uint32_t diff = current_tips - last_processed_tips;
        
        // Add tips to the current minute slot
        rain_history[current_minute_index] += diff;
        last_processed_tips = current_tips;
        total_rain_mm = current_tips * rain_calibration;

        // Recalculate rain totals immediately
        recalculate_rolling_rain();

        // Persist tips count to flash
        prefs.begin("weather", false);
        prefs.putLong("tips", current_tips);
        prefs.end();

        app_log("[Rain] Tip detected! Incremented by %u, Total tips: %u, Total rain: %s mm", diff, current_tips, String(total_rain_mm, 2).c_str());
    }

    // 2. Manage 1-minute sliding window updates
    unsigned long now_ms = millis();
    if (now_ms - last_minute_update >= 60000) {
        // Correct for possible drift (instead of resetting to now_ms)
        last_minute_update += 60000;

        // Advance sliding window minute pointer
        current_minute_index = (current_minute_index + 1) % HISTORY_MINUTES;
        
        // Clear the bin we just landed on (the oldest bin from 24h ago)
        rain_history[current_minute_index] = 0;

        // Reset daily wind gust at midnight (when the 1440-minute window rolls over to index 0)
        if (current_minute_index == 0) {
            reset_wind_gust();
        }

        // Recalculate rain statistics
        recalculate_rolling_rain();
        
        app_log("[System] Minute ticked. History index: %d | Hourly rain: %s mm | 24h rain: %s mm", 
                current_minute_index, String(rolling_rain_hour, 2).c_str(), String(rolling_rain_day, 2).c_str());
    }

    // 3. Periodic MQTT Telemetry
    static unsigned long last_mqtt_publish = 0;
    if (millis() - last_mqtt_publish >= (unsigned long)mqtt_publish_interval_s * 1000) {
        last_mqtt_publish = millis();
        if (WiFi.status() == WL_CONNECTED) {
            publish_weather_data();
        }
    }

    // 4. Wind speed: own timer, independent of I2C sensor read interval
    static unsigned long last_wind_speed_read = 0;
    if (millis() - last_wind_speed_read >= (unsigned long)wind_speed_interval_s * 1000) {
        unsigned long wind_elapsed_ms = millis() - last_wind_speed_read;
        last_wind_speed_read = millis();

        noInterrupts();
        uint32_t pulses = wind_pulse_count;
        wind_pulse_count = 0;
        interrupts();

        float elapsed_s = (float)wind_elapsed_ms / 1000.0f;
        if (elapsed_s <= 0.05f) elapsed_s = (float)wind_speed_interval_s;
        float hz = (float)pulses / elapsed_s;
        float instant_speed = hz * wind_calibration;

        // Push into speed ring buffer and compute rolling average
        int n = max(1, min(wind_speed_avg_samples, WIND_AVG_MAX_SAMPLES));
        wind_speed_buf[wind_speed_buf_idx] = instant_speed;
        wind_speed_buf_idx = (wind_speed_buf_idx + 1) % n;
        if (wind_speed_buf_count < n) wind_speed_buf_count++;

        float speed_sum = 0.0f;
        for (int i = 0; i < wind_speed_buf_count; i++) speed_sum += wind_speed_buf[i];
        wind_speed_kmh = speed_sum / wind_speed_buf_count;

        // Update daily gust using instantaneous speed (not averaged)
        if (instant_speed > wind_gust_kmh) {
            wind_gust_kmh = instant_speed;
            prefs.begin("weather", false);
            prefs.putFloat("w_gust", wind_gust_kmh);
            prefs.end();
        }
    }

    // 5. Periodic I2C environmental sensor readings (temperature, humidity, pressure, gas, lux, wind direction)
    static unsigned long last_sensor_read = 0;
    if (millis() - last_sensor_read >= (unsigned long)sensor_read_interval_s * 1000) {
        last_sensor_read = millis();
        read_environmental_sensors();
    }

    delay(10); // Small delay to yield to ESP32 background tasks
}

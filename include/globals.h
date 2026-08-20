// File: include/globals.h
#pragma once
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Preferences.h>
#include <PubSubClient.h>

// --- Firmware Version ---
#define FIRMWARE_VERSION "1.0.4"

// --- RAIN DATA CONFIGURATION ---
extern volatile uint32_t total_bucket_tips;
extern uint32_t last_processed_tips;
extern float total_rain_mm;
extern float rolling_rain_hour;
extern float rolling_rain_day;
extern bool is_raining;

// --- ENVIRONMENTAL SENSORS DATA ---
extern bool has_aht20;
extern bool has_bmp280;
extern float temperature_c;
extern float humidity_pct;
extern float pressure_hpa;

// --- ENS160 AIR QUALITY SENSOR DATA ---
extern bool has_ens160;
extern uint16_t ens160_tvoc;
extern uint16_t ens160_eco2;
extern uint8_t ens160_aqi;
void reset_ens160_baseline();
void get_ens160_resistances(uint32_t& rs0, uint32_t& rs1, uint32_t& rs2, uint32_t& rs3);

// --- ANEMOMETER (WIND SPEED) SENSOR DATA ---
extern float wind_speed_kmh;
extern float wind_gust_kmh;

// --- AS5600 WIND DIRECTION SENSOR DATA ---
extern bool has_as5600;
extern float wind_dir_deg;

// --- BH1750 LUX SENSOR DATA ---
extern bool has_bh1750;
extern float lux;
extern float lux_cal_factor;
extern float unfiltered_lux_ref;

// --- 1-MINUTE SLIDING WINDOW ARRAY (24h = 1440 minutes) ---
#define HISTORY_MINUTES 1440
extern uint16_t rain_history[HISTORY_MINUTES];
extern int current_minute_index;
extern unsigned long last_minute_update;

// --- DYNAMIC SYSTEM CONFIGURATION ---
extern String hostname;
extern bool use_dhcp;
extern bool use_imperial;
extern bool ui_compact;

extern String wifi_ssid;
extern String wifi_pass;
extern String wifi_ip;
extern String wifi_gw;
extern String wifi_nm;
extern float rain_calibration;  // mm of rain per tip (e.g. 0.2794)
extern uint32_t rain_debounce_ms; // software debounce in ms
extern int rain_sensor_pin;       // GPIO pin for rain sensor (e.g. 14)
extern int i2c_sda_pin;
extern int i2c_scl_pin;
extern int i2c_clock_khz;
extern int altitude_m;
extern float temp_offset;
extern float hum_offset;
extern float press_offset;
extern bool debug_logs_enabled;
extern int mqtt_publish_interval_s;
extern int sensor_read_interval_s;
extern int mqtt_decimals;
extern String dns_primary;
extern String dns_secondary;
extern String ntp_server;
extern int timezone_offset_h;
extern bool use_dst;
void apply_time_zone_config();

extern int wind_sensor_pin;
extern float wind_calibration;
extern int wind_radius_mm;
extern int wind_magnets;
extern float wind_factor;
extern uint32_t wind_debounce_ms;
extern int wind_dir_offset;
extern int wind_speed_avg_samples;
extern int wind_dir_avg_samples;
extern int wind_speed_interval_s;

#define WIND_AVG_MAX_SAMPLES 60
extern int wind_speed_buf_idx;
extern int wind_dir_buf_idx;
extern int wind_speed_buf_count;
extern int wind_dir_buf_count;

void reset_wind_gust();
uint16_t read_as5600_raw_angle();

// --- GLOBAL INSTANCES ---
extern Preferences prefs;
extern WiFiClient espClient;
extern PubSubClient mqttClient;

// --- MQTT FORCE RECONNECT ---
extern void mqtt_force_reconnect();

// --- LOGGER RING BUFFER FOR DEBUG CONSOLE ---
#define LOG_BUFFER_SIZE 60
#define LOG_LINE_LENGTH 128
extern char log_buffer[LOG_BUFFER_SIZE][LOG_LINE_LENGTH];
extern int log_buffer_head;
extern int log_buffer_count;

// Logger function declaration
void app_log(const char* format, ...);
void clear_logs();

// --- LED Control ---
void led_blink_mqtt();

// --- Reboot & Ping ---
void request_reboot();
String get_ping_device_id();
void send_cloud_ping(String event_type);

// --- Crash Diagnostics ---
extern bool opt_in_crash_dump;
void check_and_send_crash_dump();

// --- ENS160 Control ---
void reset_ens160_baseline();

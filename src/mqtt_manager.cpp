// File: src/mqtt_manager.cpp
#include "globals.h"
#include "mqtt_manager.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

WiFiClient espClient;
PubSubClient mqttClient(espClient);

static String mqtt_host = "";
static int mqtt_port = 1883;
static String mqtt_user = "";
static String mqtt_pass = "";

static unsigned long last_reconnect_attempt = 0;
static const unsigned long reconnect_interval = 10000; // Retry every 10s non-blocking
static bool mqtt_configs_loaded = false;

void load_mqtt_config() {
    prefs.begin("weather", true);
    mqtt_host = prefs.getString("mqtt_host", "");
    mqtt_port = prefs.getInt("mqtt_port", 1883);
    mqtt_user = prefs.getString("mqtt_user", "");
    mqtt_pass = prefs.getString("mqtt_pass", "");
    prefs.end();
    mqtt_configs_loaded = true;
}

void mqtt_force_reconnect() {
    if (mqttClient.connected()) {
        app_log("Disconnecting MQTT to reload configuration...");
        mqttClient.disconnect();
    }
    mqtt_configs_loaded = false; // Force reload on next handle_mqtt
}

void publish_ha_sensor(const char* sensor_name, const char* label, const char* unit, const char* dev_class, const char* val_template, const char* component = "sensor") {
    String lower_hostname = hostname;
    lower_hostname.toLowerCase();
    
    String object_id = lower_hostname + "_" + String(sensor_name);
    String config_topic = "homeassistant/" + String(component) + "/" + object_id + "/config";
    
    JsonDocument doc;
    doc["name"] = hostname + " " + String(label);
    doc["state_topic"] = "tele/" + hostname + "/SENSOR";
    doc["value_template"] = String(val_template);
    if (unit && strlen(unit) > 0) {
        doc["unit_of_measurement"] = String(unit);
    }
    if (dev_class && strlen(dev_class) > 0) {
        doc["device_class"] = String(dev_class);
    }
    doc["unique_id"] = object_id;
    
    JsonObject device = doc["device"].to<JsonObject>();
    JsonArray ids = device["identifiers"].to<JsonArray>();
    ids.add(lower_hostname + "_device");
    device["name"] = hostname;
    device["model"] = "DIY Smart Weather Station 2026";
    device["manufacturer"] = "PeppeBytes";
    device["sw_version"] = FIRMWARE_VERSION;
    
    size_t len = measureJson(doc);
    if (mqttClient.beginPublish(config_topic.c_str(), len, true)) {
        serializeJson(doc, mqttClient);
        mqttClient.endPublish();
    }
}

void publish_ha_discovery() {
    app_log("Publishing Home Assistant MQTT Autodiscovery config...");
    
    String t_unit = use_imperial ? "°F" : "°C";
    String p_unit = use_imperial ? "inHg" : "hPa";
    String r_unit = use_imperial ? "in" : "mm";
    String w_unit = use_imperial ? "mph" : "km/h";

    // System sensors (using safe/standard classes)
    publish_ha_sensor("uptime", "Uptime", "s", "duration", "{{ value_json.uptime }}");
    publish_ha_sensor("rssi", "RSSI", "dBm", "signal_strength", "{{ value_json.rssi }}");
    publish_ha_sensor("heap", "Free Memory", "B", "data_size", "{{ value_json.heap }}");
    
    // Environmental (using safe/standard classes)
    if (has_aht20 || has_bmp280) {
        publish_ha_sensor("temperature", "Temperature", t_unit.c_str(), "temperature", "{{ value_json.temperature }}");
    }
    if (has_aht20) {
        publish_ha_sensor("humidity", "Humidity", "%", "humidity", "{{ value_json.humidity }}");
    }
    if (has_bmp280) {
        publish_ha_sensor("pressure", "Pressure", p_unit.c_str(), "pressure", "{{ value_json.pressure }}");
    }
    
    // ENS160 Air Quality (no device class for compatibility)
    if (has_ens160) {
        publish_ha_sensor("tvoc", "TVOC", "ppb", "", "{{ value_json.tvoc }}");
        publish_ha_sensor("eco2", "eCO2", "ppm", "", "{{ value_json.eco2 }}");
        publish_ha_sensor("aqi", "AQI", "", "", "{{ value_json.aqi }}");
    }
    
    // Lux (using safe/standard classes)
    if (has_bh1750) {
        publish_ha_sensor("lux", "Luminosity", "lx", "illuminance", "{{ value_json.lux }}");
    }
    
    // Wind (no device class for compatibility)
    publish_ha_sensor("wind_speed", "Wind Speed", w_unit.c_str(), "", "{{ value_json.wind_speed }}");
    publish_ha_sensor("wind_speed_ms", "Wind Speed (m/s)", "m/s", "", "{{ value_json.wind_speed_ms }}");
    publish_ha_sensor("wind_speed_kt", "Wind Speed (knots)", "kt", "", "{{ value_json.wind_speed_kt }}");

    publish_ha_sensor("wind_gust", "Wind Gust", w_unit.c_str(), "", "{{ value_json.wind_gust }}");
    publish_ha_sensor("wind_gust_ms", "Wind Gust (m/s)", "m/s", "", "{{ value_json.wind_gust_ms }}");
    publish_ha_sensor("wind_gust_kt", "Wind Gust (knots)", "kt", "", "{{ value_json.wind_gust_kt }}");

    if (has_as5600) {
        publish_ha_sensor("wind_direction", "Wind Direction", "°", "", "{{ value_json.wind_direction }}");
    }
    
    // Rain (no device class for compatibility)
    publish_ha_sensor("total_rain", "Total Rain", r_unit.c_str(), "", "{{ value_json.total_rain_mm }}");
    publish_ha_sensor("hourly_rain", "Hourly Rain", r_unit.c_str(), "", "{{ value_json.hourly_rain_mm }}");
    publish_ha_sensor("daily_rain", "Daily Rain", r_unit.c_str(), "", "{{ value_json.daily_rain_mm }}");
    
    // Raining binary sensor (using safe/standard classes)
    publish_ha_sensor("is_raining", "Is Raining", "", "moisture", "{{ 'ON' if value_json.is_raining else 'OFF' }}", "binary_sensor");
    
    app_log("Home Assistant Autodiscovery config published successfully.");
}

bool connect_mqtt() {
    if (mqtt_host == "") return false;

    app_log("Attempting to connect to MQTT broker %s:%d...", mqtt_host.c_str(), mqtt_port);
    
    String clientId = hostname + "_" + String(ESP.getChipId(), HEX);
    
    bool connected = false;
    if (mqtt_user.length() > 0) {
        connected = mqttClient.connect(clientId.c_str(), mqtt_user.c_str(), mqtt_pass.c_str());
    } else {
        connected = mqttClient.connect(clientId.c_str());
    }

    if (connected) {
        app_log("Successfully connected to MQTT broker!");
        // We can publish a birth message
        String topic = "tele/" + hostname + "/LWT";
        mqttClient.publish(topic.c_str(), "Online", true);
        publish_ha_discovery();
    } else {
        app_log("MQTT connection failed, rc=%d", mqttClient.state());
    }
    return connected;
}

void setup_mqtt() {
    load_mqtt_config();
    if (mqtt_host != "") {
        mqttClient.setServer(mqtt_host.c_str(), mqtt_port);
    }
}

void handle_mqtt() {
    if (!mqtt_configs_loaded) {
        load_mqtt_config();
        if (mqtt_host != "") {
            mqttClient.setServer(mqtt_host.c_str(), mqtt_port);
        }
    }

    if (mqtt_host == "") return;

    if (!mqttClient.connected()) {
        unsigned long now = millis();
        if (now - last_reconnect_attempt > reconnect_interval) {
            last_reconnect_attempt = now;
            if (WiFi.status() == WL_CONNECTED) {
                connect_mqtt();
            }
        }
    } else {
        mqttClient.loop();
    }
}

void publish_weather_data() {
    if (!mqttClient.connected()) return;

    float r_mult = use_imperial ? (1.0f / 25.4f) : 1.0f;
    float w_mult = use_imperial ? 0.621371f : 1.0f;

    JsonDocument doc;
    doc["use_imperial"] = use_imperial;
    doc["uptime"] = millis() / 1000;
    doc["heap"] = ESP.getFreeHeap();
    doc["tips"] = total_bucket_tips;
    doc["total_rain_mm"] = serialized(String(total_rain_mm * r_mult, mqtt_decimals));
    doc["hourly_rain_mm"] = serialized(String(rolling_rain_hour * r_mult, mqtt_decimals));
    doc["daily_rain_mm"] = serialized(String(rolling_rain_day * r_mult, mqtt_decimals));
    doc["is_raining"] = is_raining;
    doc["rssi"] = WiFi.RSSI();
    doc["ip"] = WiFi.localIP().toString();

    if (has_aht20 || has_bmp280) {
        float temp_val = use_imperial ? (temperature_c * 1.8f + 32.0f) : temperature_c;
        doc["temperature"] = serialized(String(temp_val, mqtt_decimals));
    } else {
        doc["temperature"] = nullptr;
    }
    if (has_aht20) {
        doc["humidity"] = serialized(String(humidity_pct, mqtt_decimals));
    } else {
        doc["humidity"] = nullptr;
    }
    if (has_bmp280) {
        float press_val = use_imperial ? (pressure_hpa * 0.02953f) : pressure_hpa;
        doc["pressure"] = serialized(String(press_val, mqtt_decimals));
    } else {
        doc["pressure"] = nullptr;
    }

    if (has_ens160) {
        doc["tvoc"] = ens160_tvoc;
        doc["eco2"] = ens160_eco2;
        doc["aqi"] = ens160_aqi;
    } else {
        doc["tvoc"] = nullptr;
        doc["eco2"] = nullptr;
        doc["aqi"] = nullptr;
    }

    doc["wind_speed"] = serialized(String(wind_speed_kmh * w_mult, mqtt_decimals));
    doc["wind_speed_ms"] = serialized(String(wind_speed_kmh / 3.6f, mqtt_decimals));
    doc["wind_speed_kt"] = serialized(String(wind_speed_kmh * 0.539957f, mqtt_decimals));

    doc["wind_gust"] = serialized(String(wind_gust_kmh * w_mult, mqtt_decimals));
    doc["wind_gust_ms"] = serialized(String(wind_gust_kmh / 3.6f, mqtt_decimals));
    doc["wind_gust_kt"] = serialized(String(wind_gust_kmh * 0.539957f, mqtt_decimals));
    if (has_as5600) {
        doc["wind_direction"] = serialized(String(wind_dir_deg, mqtt_decimals));
    } else {
        doc["wind_direction"] = nullptr;
    }

    if (has_bh1750) {
        doc["lux"] = serialized(String(lux, mqtt_decimals));
    } else {
        doc["lux"] = nullptr;
    }

    String topic = "tele/" + hostname + "/SENSOR";
    size_t len = measureJson(doc);
    
    app_log("Publishing MQTT data to %s...", topic.c_str());
    if (mqttClient.beginPublish(topic.c_str(), len, false)) {
        serializeJson(doc, mqttClient);
        if (mqttClient.endPublish()) {
            app_log("Data published successfully.");
            led_blink_mqtt(); // Double blink to indicate successful MQTT transmission
        } else {
            app_log("Error ending MQTT publish.");
        }
    } else {
        app_log("Error beginning MQTT publish.");
    }
}

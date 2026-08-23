#include "weather_services.h"

#if WEATHER_UPLOAD_ANY
#include <ESP8266WiFi.h>
#include <time.h>

#include "globals.h"
#include "upload_metrics.h"
#include "weather_observation.h"
#endif
#if WEATHER_UPLOAD_WUNDERGROUND || WEATHER_UPLOAD_PWSWEATHER
#include "wu_uploader.h"
#endif

namespace {
#if WEATHER_UPLOAD_ANY
struct ServiceConfig {
    bool enabled;
    String station_id;
    String station_key;
    uint16_t interval_seconds;
};

struct ServiceRuntime {
    unsigned long last_attempt_ms;
    time_t last_attempt_utc;
    time_t last_success_utc;
    int last_http_status;
    String last_message;
    bool test_requested;
};

#if WEATHER_UPLOAD_WUNDERGROUND
ServiceConfig wunderground = {};
ServiceRuntime wunderground_runtime = {};
#endif
#if WEATHER_UPLOAD_PWSWEATHER
ServiceConfig pwsweather = {};
ServiceRuntime pwsweather_runtime = {};
#endif
UploadMetricsTracker upload_metrics;

#if WEATHER_UPLOAD_WUNDERGROUND
const char* WU_URL = "https://weatherstation.wunderground.com/weatherstation/updateweatherstation.php";
#endif
#if WEATHER_UPLOAD_PWSWEATHER
const char* PWS_URL = "https://pwsupdate.pwsweather.com/api/v1/submitwx";
#endif

uint16_t safe_interval(int interval) {
    return static_cast<uint16_t>(constrain(interval, 60, 3600));
}

bool any_upload_service_enabled() {
    bool enabled = false;
#if WEATHER_UPLOAD_WUNDERGROUND
    enabled = enabled || wunderground.enabled;
#endif
#if WEATHER_UPLOAD_PWSWEATHER
    enabled = enabled || pwsweather.enabled;
#endif
    return enabled;
}

void activate_upload_metrics() {
    if (upload_metrics.active()) return;
    upload_metrics.begin(total_bucket_tips, millis());
    app_log("[Weather Services] Upload-only rain and gust tracking enabled.");
}

void persist_upload_rain() {
    prefs.begin("weather", false);
    prefs.putInt("upl_day", upload_metrics.day_key());
    prefs.putLong("upl_rain", upload_metrics.rain_today_tips());
    prefs.end();
}

void update_upload_metrics() {
    if (!upload_metrics.active()) return;
    upload_metrics.advance_time(millis());

    time_t now = time(nullptr);
    if (now < 1700000000UL) return;
    struct tm local_now;
    localtime_r(&now, &local_now);
    const int day_key = (local_now.tm_year + 1900) * 1000 + local_now.tm_yday;

    if (upload_metrics.day_key() < 0) {
        prefs.begin("weather", true);
        const int stored_day = prefs.getInt("upl_day", -1);
        const uint32_t stored_tips = prefs.getLong("upl_rain", 0);
        prefs.end();
        upload_metrics.set_day(day_key, stored_day == day_key ? stored_tips : 0);
        if (stored_day != day_key) persist_upload_rain();
    } else if (upload_metrics.update_day(day_key)) {
        persist_upload_rain();
    }

    if (upload_metrics.observe_total_tips(total_bucket_tips) > 0) {
        persist_upload_rain();
    }
}

WeatherObservation capture_observation() {
    WeatherObservationInput input = {};
    input.timestamp_utc = time(nullptr);
    input.has_temperature = has_aht20 || has_bmp280;
    input.has_humidity = has_aht20;
    input.has_pressure = has_bmp280;
    input.has_wind_direction = has_as5600;
    input.temperature_c = temperature_c;
    input.humidity_pct = humidity_pct;
    input.pressure_hpa = pressure_hpa;
    input.wind_speed_kmh = wind_speed_kmh;
    input.wind_direction_deg = wind_dir_deg;
    input.wind_gust_10m_kmh = upload_metrics.gust_10m_kmh();
    input.rain_hour_mm = rolling_rain_hour;
    input.rain_today_mm = upload_metrics.rain_today_tips() * rain_calibration;
    return make_weather_observation(input);
}

#if WEATHER_UPLOAD_WUNDERGROUND || WEATHER_UPLOAD_PWSWEATHER
void run_service(const char* name,
                 const char* url,
                 const ServiceConfig& config,
                 ServiceRuntime& runtime,
                 const WeatherObservation& observation) {
    const unsigned long interval_ms = static_cast<unsigned long>(config.interval_seconds) * 1000UL;
    const unsigned long now_ms = millis();
    bool due = runtime.last_attempt_ms == 0 || now_ms - runtime.last_attempt_ms >= interval_ms;
    if (!runtime.test_requested && (!config.enabled || !due)) return;
    runtime.test_requested = false;
    runtime.last_attempt_ms = now_ms;
    runtime.last_attempt_utc = observation.timestamp_utc;

    if (config.station_id.length() == 0 || config.station_key.length() == 0) {
        runtime.last_http_status = 0;
        runtime.last_message = "station ID or key missing";
        app_log("[Weather Services] %s skipped: station ID or key missing.", name);
        return;
    }

    WuUploadResult result = upload_wu_observation(
        name, url, config.station_id, config.station_key, observation);
    runtime.last_http_status = result.http_status;
    runtime.last_message = result.message;
    if (result.success) runtime.last_success_utc = observation.timestamp_utc;
}
#endif

void append_service(JsonObject target,
                    const ServiceConfig& config,
                    bool include_secrets) {
    target["enabled"] = config.enabled;
    target["station_id"] = config.station_id;
    target["interval"] = config.interval_seconds;
    target["has_key"] = config.station_key.length() > 0;
    if (include_secrets) target["station_key"] = config.station_key;
}

void save_service(JsonVariantConst source,
                  ServiceConfig& config,
                  const char* enabled_key,
                  const char* id_key,
                  const char* secret_key,
                  const char* interval_key,
                  Preferences& settings) {
    if (source.isNull()) return;
    config.enabled = source["enabled"] | false;
    config.station_id = source["station_id"] | "";
    config.interval_seconds = safe_interval(source["interval"] | 60);
    String new_key = source["station_key"] | "";
    if (new_key.length() > 0) config.station_key = new_key;

    settings.putBool(enabled_key, config.enabled);
    settings.putString(id_key, config.station_id);
    settings.putString(secret_key, config.station_key);
    settings.putInt(interval_key, config.interval_seconds);
}

void append_runtime(JsonObject target, const ServiceRuntime& runtime) {
    target["last_attempt"] = runtime.last_attempt_utc;
    target["last_success"] = runtime.last_success_utc;
    target["http_status"] = runtime.last_http_status;
    target["message"] = runtime.last_message;
}
#endif
}

void setup_weather_services() {
#if WEATHER_UPLOAD_ANY
    prefs.begin("weather", true);
#if WEATHER_UPLOAD_WUNDERGROUND
    wunderground.enabled = prefs.getBool("wug_on", false);
    wunderground.station_id = prefs.getString("wug_id", "");
    wunderground.station_key = prefs.getString("wug_key", "");
    wunderground.interval_seconds = safe_interval(prefs.getInt("wug_int", 60));
#endif
#if WEATHER_UPLOAD_PWSWEATHER
    pwsweather.enabled = prefs.getBool("pws_on", false);
    pwsweather.station_id = prefs.getString("pws_id", "");
    pwsweather.station_key = prefs.getString("pws_key", "");
    pwsweather.interval_seconds = safe_interval(prefs.getInt("pws_int", 60));
#endif
    prefs.end();

    if (any_upload_service_enabled()) activate_upload_metrics();

#if WEATHER_UPLOAD_PWSWEATHER
    // Delay PWSWeather by 30 seconds to reduce peak heap use when another
    // destination also starts at boot.
    pwsweather_runtime.last_attempt_ms = millis() -
        static_cast<unsigned long>(pwsweather.interval_seconds - 30) * 1000UL;
#endif
#endif
}

void handle_weather_services() {
#if WEATHER_UPLOAD_ANY
    update_upload_metrics();
    if (WiFi.status() != WL_CONNECTED || time(nullptr) < 1700000000UL) return;
    WeatherObservation observation = capture_observation();
#if WEATHER_UPLOAD_WUNDERGROUND
    run_service("Weather Underground", WU_URL, wunderground,
                wunderground_runtime, observation);
#endif
#if WEATHER_UPLOAD_PWSWEATHER
    // Avoid back-to-back TLS allocations unless PWSWeather is explicitly due.
    run_service("PWSWeather", PWS_URL, pwsweather,
                pwsweather_runtime, observation);
#endif
#endif
}

void record_weather_service_wind_sample(float instant_speed_kmh) {
#if WEATHER_UPLOAD_ANY
    upload_metrics.record_wind_sample(instant_speed_kmh);
#else
    (void)instant_speed_kmh;
#endif
}

void append_weather_services_config(JsonDocument& doc, bool include_secrets) {
    JsonObject services = doc["weather_services"].to<JsonObject>();
#if WEATHER_UPLOAD_WUNDERGROUND
    append_service(services["wunderground"].to<JsonObject>(), wunderground, include_secrets);
#endif
#if WEATHER_UPLOAD_PWSWEATHER
    append_service(services["pwsweather"].to<JsonObject>(), pwsweather, include_secrets);
#endif
#if !WEATHER_UPLOAD_ANY
    (void)include_secrets;
#endif
}

void save_weather_services_config(JsonVariantConst config) {
#if WEATHER_UPLOAD_ANY
    if (config.isNull()) return;
    Preferences settings;
    settings.begin("weather", false);
#if WEATHER_UPLOAD_WUNDERGROUND
    save_service(config["wunderground"], wunderground,
                 "wug_on", "wug_id", "wug_key", "wug_int", settings);
#endif
#if WEATHER_UPLOAD_PWSWEATHER
    save_service(config["pwsweather"], pwsweather,
                 "pws_on", "pws_id", "pws_key", "pws_int", settings);
#endif
    settings.end();
#else
    (void)config;
#endif
}

bool queue_weather_service_test(WeatherServiceId service) {
#if WEATHER_UPLOAD_WUNDERGROUND
    if (service == WeatherServiceId::Wunderground) {
        activate_upload_metrics();
        wunderground_runtime.test_requested = true;
        return true;
    }
#endif
#if WEATHER_UPLOAD_PWSWEATHER
    if (service == WeatherServiceId::PwsWeather) {
        activate_upload_metrics();
        pwsweather_runtime.test_requested = true;
        return true;
    }
#endif
    (void)service;
    return false;
}

void append_weather_services_status(JsonDocument& doc) {
    JsonObject services = doc["weather_services"].to<JsonObject>();
#if WEATHER_UPLOAD_WUNDERGROUND
    append_runtime(services["wunderground"].to<JsonObject>(), wunderground_runtime);
#endif
#if WEATHER_UPLOAD_PWSWEATHER
    append_runtime(services["pwsweather"].to<JsonObject>(), pwsweather_runtime);
#endif
}

#include "weather_services.h"

#if WEATHER_UPLOAD_ANY
#include <ESP8266WiFi.h>
#include <math.h>
#include <time.h>

#include "globals.h"
#if WEATHER_UPLOAD_AWEKAS
#include "awekas_uploader.h"
#endif
#include "upload_metrics.h"
#if WEATHER_UPLOAD_CWOP
#include "cwop_uploader.h"
#endif
#include "weather_observation.h"
#if WEATHER_UPLOAD_WEATHERCLOUD
#include "weathercloud_uploader.h"
#endif
#if WEATHER_UPLOAD_WINDY
#include "windy_uploader.h"
#endif
#if WEATHER_UPLOAD_WOW_BE
#include "wow_be_uploader.h"
#endif
#endif
#if WEATHER_UPLOAD_WUNDERGROUND || WEATHER_UPLOAD_PWSWEATHER
#include "wu_uploader.h"
#endif

namespace {
#if WEATHER_UPLOAD_ANY
struct SensorFlags {
    bool temp = true;
    bool hum = true;
    bool press = true;
    bool wind = true;
    bool rain = true;

    uint8_t to_mask() const {
        uint8_t m = 0;
        if (temp)  m |= 0x01;
        if (hum)   m |= 0x02;
        if (press) m |= 0x04;
        if (wind)  m |= 0x08;
        if (rain)  m |= 0x10;
        return m;
    }

    void from_mask(uint8_t m) {
        temp  = (m & 0x01) != 0;
        hum   = (m & 0x02) != 0;
        press = (m & 0x04) != 0;
        wind  = (m & 0x08) != 0;
        rain  = (m & 0x10) != 0;
    }
};

WeatherObservation apply_sensor_flags(WeatherObservation obs, const SensorFlags& flags) {
    if (!flags.temp) {
        obs.has_temperature = false;
        obs.has_dew_point = false;
    }
    if (!flags.hum) {
        obs.has_humidity = false;
        obs.has_dew_point = false;
    }
    if (!flags.press) {
        obs.has_pressure = false;
    }
    if (!flags.wind) {
        obs.has_wind_speed = false;
        obs.has_wind_direction = false;
        obs.has_wind_gust = false;
    }
    if (!flags.rain) {
        obs.has_rain_hour = false;
        obs.has_rain_24h = false;
        obs.has_rain_today = false;
    }
    return obs;
}

struct ServiceConfig {
    bool enabled;
    String station_id;
    String station_key;
    uint16_t interval_seconds;
    SensorFlags sensors;
};

struct ServiceRuntime {
    unsigned long last_attempt_ms;
    time_t last_attempt_utc;
    time_t last_success_utc;
    int last_http_status;
    String last_message;
    bool test_requested;
};

#if WEATHER_UPLOAD_CWOP
struct CwopConfig {
    bool enabled;
    String station_id;
    String passcode;
    float latitude;
    float longitude;
    uint16_t interval_seconds;
    String server;
    uint16_t port;
    SensorFlags sensors;
};
#endif

#if WEATHER_UPLOAD_AWEKAS
struct AwekasConfig {
    bool enabled;
    String username;
    String password;
    float latitude;
    float longitude;
    uint16_t interval_seconds;
    SensorFlags sensors;
};
#endif

#if WEATHER_UPLOAD_WUNDERGROUND
ServiceConfig wunderground = {};
ServiceRuntime wunderground_runtime = {};
#endif
#if WEATHER_UPLOAD_PWSWEATHER
ServiceConfig pwsweather = {};
ServiceRuntime pwsweather_runtime = {};
#endif
#if WEATHER_UPLOAD_WEATHERCLOUD
ServiceConfig weathercloud = {};
ServiceRuntime weathercloud_runtime = {};
#endif
#if WEATHER_UPLOAD_WINDY
ServiceConfig windy = {};
ServiceRuntime windy_runtime = {};
#endif
#if WEATHER_UPLOAD_AWEKAS
AwekasConfig awekas = {};
ServiceRuntime awekas_runtime = {};
#endif
#if WEATHER_UPLOAD_WOW_BE
ServiceConfig wow_be = {};
ServiceRuntime wow_be_runtime = {};
#endif
UploadMetricsTracker upload_metrics;
#if WEATHER_UPLOAD_CWOP
CwopConfig cwop = {};
ServiceRuntime cwop_runtime = {};
#endif

#if WEATHER_UPLOAD_WUNDERGROUND
const char* WU_URL = "http://weatherstation.wunderground.com/weatherstation/updateweatherstation.php";
#endif
#if WEATHER_UPLOAD_PWSWEATHER
const char* PWS_URL = "https://pwsupdate.pwsweather.com/api/v1/submitwx";
#endif

uint16_t safe_interval(int interval, int minimum = 60) {
    return static_cast<uint16_t>(constrain(interval, minimum, 3600));
}

bool any_upload_service_enabled() {
    bool enabled = false;
#if WEATHER_UPLOAD_WUNDERGROUND
    enabled = enabled || wunderground.enabled;
#endif
#if WEATHER_UPLOAD_PWSWEATHER
    enabled = enabled || pwsweather.enabled;
#endif
#if WEATHER_UPLOAD_CWOP
    enabled = enabled || cwop.enabled;
#endif
#if WEATHER_UPLOAD_WEATHERCLOUD
    enabled = enabled || weathercloud.enabled;
#endif
#if WEATHER_UPLOAD_WINDY
    enabled = enabled || windy.enabled;
#endif
#if WEATHER_UPLOAD_AWEKAS
    enabled = enabled || awekas.enabled;
#endif
#if WEATHER_UPLOAD_WOW_BE
    enabled = enabled || wow_be.enabled;
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
    input.rain_24h_mm = rolling_rain_day;
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
        name, url, config.station_id, config.station_key, apply_sensor_flags(observation, config.sensors));
    runtime.last_http_status = result.http_status;
    runtime.last_message = result.message;
    if (result.success) runtime.last_success_utc = observation.timestamp_utc;
}
#endif

void append_sensors_config(JsonObject target, const SensorFlags& flags) {
    JsonObject sens = target["sensors"].to<JsonObject>();
    sens["temp"] = flags.temp;
    sens["hum"] = flags.hum;
    sens["press"] = flags.press;
    sens["wind"] = flags.wind;
    sens["rain"] = flags.rain;
}

void parse_sensors_config(JsonVariantConst source, SensorFlags& flags) {
    if (source["sensors"].is<JsonObject>()) {
        JsonObjectConst sens = source["sensors"];
        flags.temp  = sens["temp"]  | true;
        flags.hum   = sens["hum"]   | true;
        flags.press = sens["press"] | true;
        flags.wind  = sens["wind"]  | true;
        flags.rain  = sens["rain"]  | true;
    }
}

void append_service(JsonObject target,
                    const ServiceConfig& config,
                    bool include_secrets) {
    target["enabled"] = config.enabled;
    target["station_id"] = config.station_id;
    target["interval"] = config.interval_seconds;
    target["has_key"] = config.station_key.length() > 0;
    if (include_secrets) target["station_key"] = config.station_key;
    append_sensors_config(target, config.sensors);
}

void save_service(JsonVariantConst source,
                  ServiceConfig& config,
                  const char* enabled_key,
                  const char* id_key,
                  const char* secret_key,
                  const char* interval_key,
                  const char* sens_key,
                  Preferences& settings) {
    if (source.isNull()) return;
    config.enabled = source["enabled"] | false;
    config.station_id = source["station_id"] | "";
    config.interval_seconds = safe_interval(source["interval"] | 60);
    String new_key = source["station_key"] | "";
    if (new_key.length() > 0) config.station_key = new_key;
    parse_sensors_config(source, config.sensors);

    settings.putBool(enabled_key, config.enabled);
    settings.putString(id_key, config.station_id);
    settings.putString(secret_key, config.station_key);
    settings.putInt(interval_key, config.interval_seconds);
    settings.putUChar(sens_key, config.sensors.to_mask());
}

void append_runtime(JsonObject target, const ServiceRuntime& runtime) {
    target["last_attempt"] = runtime.last_attempt_utc;
    target["last_success"] = runtime.last_success_utc;
    target["http_status"] = runtime.last_http_status;
    target["message"] = runtime.last_message;
}

#if WEATHER_UPLOAD_CWOP
void run_cwop(const WeatherObservation& observation) {
    const unsigned long interval_ms = static_cast<unsigned long>(cwop.interval_seconds) * 1000UL;
    const unsigned long now_ms = millis();
    bool due = cwop_runtime.last_attempt_ms == 0 ||
               now_ms - cwop_runtime.last_attempt_ms >= interval_ms;
    if (!cwop_runtime.test_requested && (!cwop.enabled || !due)) return;
    cwop_runtime.test_requested = false;
    cwop_runtime.last_attempt_ms = now_ms;
    cwop_runtime.last_attempt_utc = observation.timestamp_utc;

    if (cwop.station_id.length() == 0 || !isfinite(cwop.latitude) ||
        !isfinite(cwop.longitude) || cwop.latitude < -90.0f || cwop.latitude > 90.0f ||
        cwop.longitude < -180.0f || cwop.longitude > 180.0f) {
        cwop_runtime.last_message = "station ID or valid coordinates missing";
        app_log("[Weather Services] CWOP skipped: station ID or coordinates missing.");
        return;
    }

    CwopUploadResult result = upload_cwop_observation(
        cwop.server, cwop.port, cwop.station_id, cwop.passcode,
        cwop.latitude, cwop.longitude, apply_sensor_flags(observation, cwop.sensors));
    cwop_runtime.last_http_status = 0;
    cwop_runtime.last_message = result.message;
    if (result.success) cwop_runtime.last_success_utc = observation.timestamp_utc;
}
#endif

#if WEATHER_UPLOAD_WEATHERCLOUD
void run_weathercloud(const WeatherObservation& observation) {
    const unsigned long interval_ms = static_cast<unsigned long>(weathercloud.interval_seconds) * 1000UL;
    const unsigned long now_ms = millis();
    bool due = weathercloud_runtime.last_attempt_ms == 0 ||
               now_ms - weathercloud_runtime.last_attempt_ms >= interval_ms;
    if (!weathercloud_runtime.test_requested && (!weathercloud.enabled || !due)) return;
    weathercloud_runtime.test_requested = false;
    weathercloud_runtime.last_attempt_ms = now_ms;
    weathercloud_runtime.last_attempt_utc = observation.timestamp_utc;

    if (weathercloud.station_id.length() == 0 || weathercloud.station_key.length() == 0) {
        weathercloud_runtime.last_http_status = 0;
        weathercloud_runtime.last_message = "device ID or key missing";
        app_log("[Weather Services] Weathercloud skipped: device ID or key missing.");
        return;
    }

    WeathercloudUploadResult result = upload_weathercloud_observation(
        weathercloud.station_id, weathercloud.station_key, apply_sensor_flags(observation, weathercloud.sensors));
    weathercloud_runtime.last_http_status = result.http_status;
    weathercloud_runtime.last_message = result.message;
    if (result.success) weathercloud_runtime.last_success_utc = observation.timestamp_utc;
}
#endif

#if WEATHER_UPLOAD_WINDY
void run_windy(const WeatherObservation& observation) {
    const unsigned long interval_ms = static_cast<unsigned long>(windy.interval_seconds) * 1000UL;
    const unsigned long now_ms = millis();
    bool due = windy_runtime.last_attempt_ms == 0 || now_ms - windy_runtime.last_attempt_ms >= interval_ms;
    if (!windy_runtime.test_requested && (!windy.enabled || !due)) return;
    windy_runtime.test_requested = false;
    windy_runtime.last_attempt_ms = now_ms;
    windy_runtime.last_attempt_utc = observation.timestamp_utc;
    if (windy.station_id.length() == 0 || windy.station_key.length() == 0) {
        windy_runtime.last_http_status = 0;
        windy_runtime.last_message = "station ID or password missing";
        app_log("[Weather Services] Windy skipped: station ID or password missing.");
        return;
    }
    WindyUploadResult result = upload_windy_observation(
        windy.station_id, windy.station_key, apply_sensor_flags(observation, windy.sensors));
    windy_runtime.last_http_status = result.http_status;
    windy_runtime.last_message = result.message;
    if (result.success) windy_runtime.last_success_utc = observation.timestamp_utc;
}
#endif

#if WEATHER_UPLOAD_AWEKAS
void run_awekas(const WeatherObservation& observation) {
    const unsigned long interval_ms = static_cast<unsigned long>(awekas.interval_seconds) * 1000UL;
    const unsigned long now_ms = millis();
    bool due = awekas_runtime.last_attempt_ms == 0 || now_ms - awekas_runtime.last_attempt_ms >= interval_ms;
    if (!awekas_runtime.test_requested && (!awekas.enabled || !due)) return;
    awekas_runtime.test_requested = false;
    awekas_runtime.last_attempt_ms = now_ms;
    awekas_runtime.last_attempt_utc = observation.timestamp_utc;
    if (awekas.username.length() == 0 || awekas.password.length() == 0 ||
        !isfinite(awekas.latitude) || !isfinite(awekas.longitude) ||
        awekas.latitude < -90.0f || awekas.latitude > 90.0f ||
        awekas.longitude < -180.0f || awekas.longitude > 180.0f) {
        awekas_runtime.last_http_status = 0;
        awekas_runtime.last_message = "credentials or valid coordinates missing";
        app_log("[Weather Services] AWEKAS skipped: credentials or coordinates missing.");
        return;
    }
    AwekasUploadResult result = upload_awekas_observation(
        awekas.username, awekas.password, awekas.latitude, awekas.longitude, apply_sensor_flags(observation, awekas.sensors));
    awekas_runtime.last_http_status = result.http_status;
    awekas_runtime.last_message = result.message;
    if (result.success) awekas_runtime.last_success_utc = observation.timestamp_utc;
}
#endif

#if WEATHER_UPLOAD_WOW_BE
void run_wow_be(const WeatherObservation& observation) {
    const unsigned long interval_ms = static_cast<unsigned long>(wow_be.interval_seconds) * 1000UL;
    const unsigned long now_ms = millis();
    bool due = wow_be_runtime.last_attempt_ms == 0 || now_ms - wow_be_runtime.last_attempt_ms >= interval_ms;
    if (!wow_be_runtime.test_requested && (!wow_be.enabled || !due)) return;
    wow_be_runtime.test_requested = false;
    wow_be_runtime.last_attempt_ms = now_ms;
    wow_be_runtime.last_attempt_utc = observation.timestamp_utc;
    if (wow_be.station_id.length() == 0 || wow_be.station_key.length() == 0) {
        wow_be_runtime.last_http_status = 0;
        wow_be_runtime.last_message = "site ID or authentication key missing";
        app_log("[Weather Services] WOW-BE skipped: site ID or authentication key missing.");
        return;
    }
    WowBeUploadResult result = upload_wow_be_observation(
        wow_be.station_id, wow_be.station_key, apply_sensor_flags(observation, wow_be.sensors));
    wow_be_runtime.last_http_status = result.http_status;
    wow_be_runtime.last_message = result.message;
    if (result.success) wow_be_runtime.last_success_utc = observation.timestamp_utc;
}
#endif
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
    wunderground.sensors.from_mask(prefs.getUChar("wug_sens", 0x1F));
#endif
#if WEATHER_UPLOAD_PWSWEATHER
    pwsweather.enabled = prefs.getBool("pws_on", false);
    pwsweather.station_id = prefs.getString("pws_id", "");
    pwsweather.station_key = prefs.getString("pws_key", "");
    pwsweather.interval_seconds = safe_interval(prefs.getInt("pws_int", 60));
    pwsweather.sensors.from_mask(prefs.getUChar("pws_sens", 0x1F));
#endif
#if WEATHER_UPLOAD_CWOP
    cwop.enabled = prefs.getBool("cwop_on", false);
    cwop.station_id = prefs.getString("cwop_id", "");
    cwop.station_id.toUpperCase();
    cwop.passcode = prefs.getString("cwop_pass", "-1");
    cwop.latitude = prefs.getFloat("cwop_lat", NAN);
    cwop.longitude = prefs.getFloat("cwop_lon", NAN);
    cwop.interval_seconds = static_cast<uint16_t>(constrain(prefs.getInt("cwop_int", 300), 300, 3600));
    cwop.server = prefs.getString("cwop_host", "cwop.aprs.net");
    cwop.port = static_cast<uint16_t>(constrain(prefs.getInt("cwop_port", 14580), 1, 65535));
    cwop.sensors.from_mask(prefs.getUChar("cwop_sens", 0x1F));
#endif
#if WEATHER_UPLOAD_WEATHERCLOUD
    weathercloud.enabled = prefs.getBool("wcl_on", false);
    weathercloud.station_id = prefs.getString("wcl_id", "");
    weathercloud.station_key = prefs.getString("wcl_key", "");
    weathercloud.interval_seconds = safe_interval(prefs.getInt("wcl_int", 600), 600);
    weathercloud.sensors.from_mask(prefs.getUChar("wcl_sens", 0x1F));
#endif
#if WEATHER_UPLOAD_WINDY
    windy.enabled = prefs.getBool("wnd_on", false);
    windy.station_id = prefs.getString("wnd_id", "");
    windy.station_key = prefs.getString("wnd_pass", "");
    windy.interval_seconds = safe_interval(prefs.getInt("wnd_int", 300), 300);
    windy.sensors.from_mask(prefs.getUChar("wnd_sens", 0x1F));
#endif
#if WEATHER_UPLOAD_AWEKAS
    awekas.enabled = prefs.getBool("awk_on", false);
    awekas.username = prefs.getString("awk_user", "");
    awekas.password = prefs.getString("awk_pass", "");
    awekas.latitude = prefs.getFloat("awk_lat", NAN);
    awekas.longitude = prefs.getFloat("awk_lon", NAN);
    awekas.interval_seconds = safe_interval(prefs.getInt("awk_int", 300), 300);
    awekas.sensors.from_mask(prefs.getUChar("awk_sens", 0x1F));
#endif
#if WEATHER_UPLOAD_WOW_BE
    wow_be.enabled = prefs.getBool("wow_on", false);
    wow_be.station_id = prefs.getString("wow_id", "");
    wow_be.station_key = prefs.getString("wow_key", "");
    wow_be.interval_seconds = safe_interval(prefs.getInt("wow_int", 300));
    wow_be.sensors.from_mask(prefs.getUChar("wow_sens", 0x1F));
#endif
    prefs.end();

    if (any_upload_service_enabled()) activate_upload_metrics();

#if WEATHER_UPLOAD_PWSWEATHER
    // Delay PWSWeather by 30 seconds to reduce peak heap use when another
    // destination also starts at boot.
    pwsweather_runtime.last_attempt_ms = millis() -
        static_cast<unsigned long>(pwsweather.interval_seconds - 30) * 1000UL;
#endif
#if WEATHER_UPLOAD_CWOP
    cwop_runtime.last_attempt_ms = millis() -
        static_cast<unsigned long>(cwop.interval_seconds - 45) * 1000UL;
#endif
#if WEATHER_UPLOAD_WEATHERCLOUD
    weathercloud_runtime.last_attempt_ms = millis() -
        static_cast<unsigned long>(weathercloud.interval_seconds - 90) * 1000UL;
#endif
#if WEATHER_UPLOAD_WINDY
    windy_runtime.last_attempt_ms = millis() -
        static_cast<unsigned long>(windy.interval_seconds - 120) * 1000UL;
#endif
#if WEATHER_UPLOAD_AWEKAS
    awekas_runtime.last_attempt_ms = millis() -
        static_cast<unsigned long>(awekas.interval_seconds - 150) * 1000UL;
#endif
#if WEATHER_UPLOAD_WOW_BE
    wow_be_runtime.last_attempt_ms = millis() -
        static_cast<unsigned long>(wow_be.interval_seconds - 30) * 1000UL;
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
#if WEATHER_UPLOAD_CWOP
    run_cwop(observation);
#endif
#if WEATHER_UPLOAD_WEATHERCLOUD
    run_weathercloud(observation);
#endif
#if WEATHER_UPLOAD_WINDY
    run_windy(observation);
#endif
#if WEATHER_UPLOAD_AWEKAS
    run_awekas(observation);
#endif
#if WEATHER_UPLOAD_WOW_BE
    run_wow_be(observation);
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
#if WEATHER_UPLOAD_CWOP
    JsonObject cwop_json = services["cwop"].to<JsonObject>();
    cwop_json["enabled"] = cwop.enabled;
    cwop_json["station_id"] = cwop.station_id;
    cwop_json["has_passcode"] = cwop.passcode.length() > 0 && cwop.passcode != "-1";
    if (include_secrets) cwop_json["passcode"] = cwop.passcode;
    if (isfinite(cwop.latitude)) cwop_json["latitude"] = cwop.latitude;
    if (isfinite(cwop.longitude)) cwop_json["longitude"] = cwop.longitude;
    cwop_json["interval"] = cwop.interval_seconds;
    cwop_json["server"] = cwop.server;
    cwop_json["port"] = cwop.port;
    append_sensors_config(cwop_json, cwop.sensors);
#endif
#if WEATHER_UPLOAD_WEATHERCLOUD
    append_service(services["weathercloud"].to<JsonObject>(), weathercloud, include_secrets);
#endif
#if WEATHER_UPLOAD_WINDY
    append_service(services["windy"].to<JsonObject>(), windy, include_secrets);
#endif
#if WEATHER_UPLOAD_AWEKAS
    JsonObject awekas_json = services["awekas"].to<JsonObject>();
    awekas_json["enabled"] = awekas.enabled;
    awekas_json["username"] = awekas.username;
    awekas_json["has_password"] = awekas.password.length() > 0;
    if (include_secrets) awekas_json["password"] = awekas.password;
    if (isfinite(awekas.latitude)) awekas_json["latitude"] = awekas.latitude;
    if (isfinite(awekas.longitude)) awekas_json["longitude"] = awekas.longitude;
    awekas_json["interval"] = awekas.interval_seconds;
    append_sensors_config(awekas_json, awekas.sensors);
#endif
#if WEATHER_UPLOAD_WOW_BE
    append_service(services["wow_be"].to<JsonObject>(), wow_be, include_secrets);
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
                 "wug_on", "wug_id", "wug_key", "wug_int", "wug_sens", settings);
#endif
#if WEATHER_UPLOAD_PWSWEATHER
    save_service(config["pwsweather"], pwsweather,
                 "pws_on", "pws_id", "pws_key", "pws_int", "pws_sens", settings);
#endif
#if WEATHER_UPLOAD_CWOP
    JsonVariantConst cwop_json = config["cwop"];
    if (!cwop_json.isNull()) {
        cwop.enabled = cwop_json["enabled"] | false;
        cwop.station_id = cwop_json["station_id"] | "";
        cwop.station_id.toUpperCase();
        String new_passcode = cwop_json["passcode"] | "";
        if (new_passcode.length() > 0) cwop.passcode = new_passcode;
        if (cwop.passcode.length() == 0) cwop.passcode = "-1";
        cwop.latitude = cwop_json["latitude"] | NAN;
        cwop.longitude = cwop_json["longitude"] | NAN;
        cwop.interval_seconds = static_cast<uint16_t>(
            constrain(cwop_json["interval"] | 300, 300, 3600));
        cwop.server = cwop_json["server"] | "cwop.aprs.net";
        cwop.port = static_cast<uint16_t>(constrain(cwop_json["port"] | 14580, 1, 65535));
        parse_sensors_config(cwop_json, cwop.sensors);
        settings.putBool("cwop_on", cwop.enabled);
        settings.putString("cwop_id", cwop.station_id);
        settings.putString("cwop_pass", cwop.passcode);
        settings.putFloat("cwop_lat", cwop.latitude);
        settings.putFloat("cwop_lon", cwop.longitude);
        settings.putInt("cwop_int", cwop.interval_seconds);
        settings.putString("cwop_host", cwop.server);
        settings.putInt("cwop_port", cwop.port);
        settings.putUChar("cwop_sens", cwop.sensors.to_mask());
    }
#endif
#if WEATHER_UPLOAD_WEATHERCLOUD
    JsonVariantConst weathercloud_json = config["weathercloud"];
    if (!weathercloud_json.isNull()) {
        weathercloud.enabled = weathercloud_json["enabled"] | false;
        weathercloud.station_id = weathercloud_json["station_id"] | "";
        weathercloud.interval_seconds = safe_interval(weathercloud_json["interval"] | 600, 600);
        String new_key = weathercloud_json["station_key"] | "";
        if (new_key.length() > 0) weathercloud.station_key = new_key;
        parse_sensors_config(weathercloud_json, weathercloud.sensors);
        settings.putBool("wcl_on", weathercloud.enabled);
        settings.putString("wcl_id", weathercloud.station_id);
        settings.putString("wcl_key", weathercloud.station_key);
        settings.putInt("wcl_int", weathercloud.interval_seconds);
        settings.putUChar("wcl_sens", weathercloud.sensors.to_mask());
    }
#endif
#if WEATHER_UPLOAD_WINDY
    JsonVariantConst windy_json = config["windy"];
    if (!windy_json.isNull()) {
        windy.enabled = windy_json["enabled"] | false;
        windy.station_id = windy_json["station_id"] | "";
        windy.interval_seconds = safe_interval(windy_json["interval"] | 300, 300);
        String new_password = windy_json["station_key"] | "";
        if (new_password.length() > 0) windy.station_key = new_password;
        parse_sensors_config(windy_json, windy.sensors);
        settings.putBool("wnd_on", windy.enabled);
        settings.putString("wnd_id", windy.station_id);
        settings.putString("wnd_pass", windy.station_key);
        settings.putInt("wnd_int", windy.interval_seconds);
        settings.putUChar("wnd_sens", windy.sensors.to_mask());
    }
#endif
#if WEATHER_UPLOAD_AWEKAS
    JsonVariantConst awekas_json = config["awekas"];
    if (!awekas_json.isNull()) {
        awekas.enabled = awekas_json["enabled"] | false;
        awekas.username = awekas_json["username"] | "";
        String new_password = awekas_json["password"] | "";
        if (new_password.length() > 0) awekas.password = new_password;
        awekas.latitude = awekas_json["latitude"] | NAN;
        awekas.longitude = awekas_json["longitude"] | NAN;
        awekas.interval_seconds = safe_interval(awekas_json["interval"] | 300, 300);
        parse_sensors_config(awekas_json, awekas.sensors);
        settings.putBool("awk_on", awekas.enabled);
        settings.putString("awk_user", awekas.username);
        settings.putString("awk_pass", awekas.password);
        settings.putFloat("awk_lat", awekas.latitude);
        settings.putFloat("awk_lon", awekas.longitude);
        settings.putInt("awk_int", awekas.interval_seconds);
        settings.putUChar("awk_sens", awekas.sensors.to_mask());
    }
#endif
#if WEATHER_UPLOAD_WOW_BE
    JsonVariantConst wow_be_json = config["wow_be"];
    if (!wow_be_json.isNull()) {
        wow_be.enabled = wow_be_json["enabled"] | false;
        wow_be.station_id = wow_be_json["station_id"] | "";
        wow_be.interval_seconds = safe_interval(wow_be_json["interval"] | 300);
        String new_key = wow_be_json["station_key"] | "";
        if (new_key.length() > 0) wow_be.station_key = new_key;
        parse_sensors_config(wow_be_json, wow_be.sensors);
        settings.putBool("wow_on", wow_be.enabled);
        settings.putString("wow_id", wow_be.station_id);
        settings.putString("wow_key", wow_be.station_key);
        settings.putInt("wow_int", wow_be.interval_seconds);
        settings.putUChar("wow_sens", wow_be.sensors.to_mask());
    }
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
#if WEATHER_UPLOAD_CWOP
    if (service == WeatherServiceId::Cwop) {
        activate_upload_metrics();
        cwop_runtime.test_requested = true;
        return true;
    }
#endif
#if WEATHER_UPLOAD_WEATHERCLOUD
    if (service == WeatherServiceId::Weathercloud) {
        activate_upload_metrics();
        weathercloud_runtime.test_requested = true;
        return true;
    }
#endif
#if WEATHER_UPLOAD_WINDY
    if (service == WeatherServiceId::Windy) {
        activate_upload_metrics();
        windy_runtime.test_requested = true;
        return true;
    }
#endif
#if WEATHER_UPLOAD_AWEKAS
    if (service == WeatherServiceId::Awekas) {
        activate_upload_metrics();
        awekas_runtime.test_requested = true;
        return true;
    }
#endif
#if WEATHER_UPLOAD_WOW_BE
    if (service == WeatherServiceId::WowBe) {
        activate_upload_metrics();
        wow_be_runtime.test_requested = true;
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
#if WEATHER_UPLOAD_CWOP
    append_runtime(services["cwop"].to<JsonObject>(), cwop_runtime);
#endif
#if WEATHER_UPLOAD_WEATHERCLOUD
    append_runtime(services["weathercloud"].to<JsonObject>(), weathercloud_runtime);
#endif
#if WEATHER_UPLOAD_WINDY
    append_runtime(services["windy"].to<JsonObject>(), windy_runtime);
#endif
#if WEATHER_UPLOAD_AWEKAS
    append_runtime(services["awekas"].to<JsonObject>(), awekas_runtime);
#endif
#if WEATHER_UPLOAD_WOW_BE
    append_runtime(services["wow_be"].to<JsonObject>(), wow_be_runtime);
#endif
}

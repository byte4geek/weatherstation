#include "weathercloud_formatter.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

namespace {
bool append_text(char* output, size_t output_size, size_t& used, const char* text) {
    const size_t length = strlen(text);
    if (used + length >= output_size) return false;
    memcpy(output + used, text, length);
    used += length;
    output[used] = '\0';
    return true;
}

bool append_encoded(char* output, size_t output_size, size_t& used, const char* value) {
    static const char hex[] = "0123456789ABCDEF";
    if (!value) return false;
    for (const unsigned char* cursor = reinterpret_cast<const unsigned char*>(value);
         *cursor; ++cursor) {
        const unsigned char c = *cursor;
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
            char plain[2] = {static_cast<char>(c), '\0'};
            if (!append_text(output, output_size, used, plain)) return false;
        } else {
            char escaped[4] = {'%', hex[c >> 4], hex[c & 0x0f], '\0'};
            if (!append_text(output, output_size, used, escaped)) return false;
        }
    }
    return true;
}

bool append_parameter(char* output, size_t output_size, size_t& used,
                      const char* name, const char* value) {
    return append_text(output, output_size, used, used == 0 ? "?" : "&") &&
           append_text(output, output_size, used, name) &&
           append_text(output, output_size, used, "=") &&
           append_encoded(output, output_size, used, value);
}

bool append_number(char* output, size_t output_size, size_t& used,
                   const char* name, long value) {
    char text[24];
    snprintf(text, sizeof(text), "%ld", value);
    return append_parameter(output, output_size, used, name, text);
}
}

bool build_weathercloud_query(char* output,
                              size_t output_size,
                              const char* device_id,
                              const char* device_key,
                              const char* firmware_version,
                              const WeatherObservation& observation) {
    if (!output || output_size == 0 || !device_id || !device_id[0] ||
        !device_key || !device_key[0] || !firmware_version || !firmware_version[0] ||
        observation.timestamp_utc <= 0) return false;

    output[0] = '\0';
    size_t used = 0;
    struct tm utc;
    gmtime_r(&observation.timestamp_utc, &utc);
    char date[9];
    char clock[5];
    if (strftime(date, sizeof(date), "%Y%m%d", &utc) == 0 ||
        strftime(clock, sizeof(clock), "%H%M", &utc) == 0) return false;

    if (!append_parameter(output, output_size, used, "wid", device_id) ||
        !append_parameter(output, output_size, used, "key", device_key) ||
        !append_parameter(output, output_size, used, "date", date) ||
        !append_parameter(output, output_size, used, "time", clock)) return false;

    if (observation.has_temperature &&
        !append_number(output, output_size, used, "temp", lroundf(observation.temperature_c * 10.0f))) return false;
    if (observation.has_humidity &&
        !append_number(output, output_size, used, "hum", lroundf(observation.humidity_pct))) return false;
    if (observation.has_wind_direction &&
        !append_number(output, output_size, used, "wdir", lroundf(observation.wind_direction_deg))) return false;
    if (observation.has_wind_speed &&
        !append_number(output, output_size, used, "wspd", lroundf(observation.wind_speed_kmh / 3.6f * 10.0f))) return false;
    if (observation.has_wind_gust &&
        !append_number(output, output_size, used, "wspdhi", lroundf(observation.wind_gust_10m_kmh / 3.6f * 10.0f))) return false;
    if (observation.has_pressure &&
        !append_number(output, output_size, used, "bar", lroundf(observation.pressure_hpa * 10.0f))) return false;
    if (observation.has_rain_today &&
        !append_number(output, output_size, used, "rain", lroundf(observation.rain_today_mm * 10.0f))) return false;
    if (observation.has_dew_point &&
        !append_number(output, output_size, used, "dew", lroundf(observation.dew_point_c * 10.0f))) return false;
    if (observation.has_solar_radiation &&
        !append_number(output, output_size, used, "solarrad", lroundf(observation.solar_radiation_wm2 * 10.0f))) return false;

    return append_parameter(output, output_size, used, "ver", firmware_version);
}

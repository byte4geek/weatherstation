#include "windy_formatter.h"

#include <stdio.h>
#include <string.h>

namespace {
bool append(char* output, size_t output_size, size_t& used, const char* text) {
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
    for (const unsigned char* cursor = reinterpret_cast<const unsigned char*>(value); *cursor; ++cursor) {
        const unsigned char c = *cursor;
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
            char plain[2] = {static_cast<char>(c), '\0'};
            if (!append(output, output_size, used, plain)) return false;
        } else {
            char escaped[4] = {'%', hex[c >> 4], hex[c & 0x0f], '\0'};
            if (!append(output, output_size, used, escaped)) return false;
        }
    }
    return true;
}

bool parameter(char* output, size_t output_size, size_t& used,
               const char* name, const char* value) {
    return append(output, output_size, used, used == 0 ? "?" : "&") &&
           append(output, output_size, used, name) && append(output, output_size, used, "=") &&
           append_encoded(output, output_size, used, value);
}

bool decimal_parameter(char* output, size_t output_size, size_t& used,
                       const char* name, float value, unsigned int decimals) {
    char text[32];
    snprintf(text, sizeof(text), decimals == 0 ? "%.0f" : decimals == 1 ? "%.1f" : "%.2f", value);
    return parameter(output, output_size, used, name, text);
}
}

bool build_windy_query(char* output,
                       size_t output_size,
                       const char* station_id,
                       const char* firmware_version,
                       const WeatherObservation& observation) {
    if (!output || output_size == 0 || !station_id || !station_id[0] ||
        !firmware_version || !firmware_version[0] || observation.timestamp_utc <= 0) return false;
    output[0] = '\0';
    size_t used = 0;
    char timestamp[24];
    snprintf(timestamp, sizeof(timestamp), "%lld", static_cast<long long>(observation.timestamp_utc));
    if (!parameter(output, output_size, used, "id", station_id) ||
        !parameter(output, output_size, used, "ts", timestamp)) return false;

    if (observation.has_temperature &&
        !decimal_parameter(output, output_size, used, "temp", observation.temperature_c, 2)) return false;
    if (observation.has_dew_point &&
        !decimal_parameter(output, output_size, used, "dewpoint", observation.dew_point_c, 2)) return false;
    if (observation.has_humidity &&
        !decimal_parameter(output, output_size, used, "humidity", observation.humidity_pct, 1)) return false;
    if (observation.has_wind_speed &&
        !decimal_parameter(output, output_size, used, "wind", observation.wind_speed_kmh / 3.6f, 2)) return false;
    if (observation.has_wind_gust &&
        !decimal_parameter(output, output_size, used, "gust", observation.wind_gust_10m_kmh / 3.6f, 2)) return false;
    if (observation.has_wind_direction &&
        !decimal_parameter(output, output_size, used, "winddir", observation.wind_direction_deg, 0)) return false;
    if (observation.has_pressure &&
        !decimal_parameter(output, output_size, used, "pressure", observation.pressure_hpa * 100.0f, 0)) return false;
    if (observation.has_rain_hour &&
        !decimal_parameter(output, output_size, used, "precip", observation.rain_hour_mm, 2)) return false;

    char software[80];
    snprintf(software, sizeof(software), "byte4geek-weatherstation-%s", firmware_version);
    return parameter(output, output_size, used, "softwaretype", software);
}

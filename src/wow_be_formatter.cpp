#include "wow_be_formatter.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

namespace {
bool append(char* output, size_t output_size, size_t& used, const char* text) {
    const size_t length = strlen(text);
    if (used + length >= output_size) return false;
    memcpy(output + used, text, length);
    used += length;
    output[used] = '\0';
    return true;
}

bool encoded(char* output, size_t output_size, size_t& used, const char* value) {
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
           encoded(output, output_size, used, value);
}

bool number(char* output, size_t output_size, size_t& used,
            const char* name, float value, unsigned int decimals) {
    char text[32];
    snprintf(text, sizeof(text), decimals == 0 ? "%.0f" : decimals == 1 ? "%.1f" :
             decimals == 2 ? "%.2f" : decimals == 3 ? "%.3f" : "%.4f", value);
    return parameter(output, output_size, used, name, text);
}
}

bool build_wow_be_query(char* output,
                        size_t output_size,
                        const char* site_id,
                        const char* authentication_key,
                        const char* firmware_version,
                        const WeatherObservation& observation) {
    if (!output || output_size == 0 || !site_id || !site_id[0] ||
        !authentication_key || !authentication_key[0] || !firmware_version ||
        !firmware_version[0] || observation.timestamp_utc <= 0) return false;
    output[0] = '\0';
    size_t used = 0;
    struct tm utc;
    gmtime_r(&observation.timestamp_utc, &utc);
    char date_utc[20];
    if (strftime(date_utc, sizeof(date_utc), "%Y-%m-%d %H:%M:%S", &utc) == 0) return false;
    char software[80];
    snprintf(software, sizeof(software), "byte4geek-weatherstation-%s", firmware_version);

    if (!parameter(output, output_size, used, "action", "updateraw") ||
        !parameter(output, output_size, used, "siteid", site_id) ||
        !parameter(output, output_size, used, "siteAuthenticationKey", authentication_key) ||
        !parameter(output, output_size, used, "dateutc", date_utc) ||
        !parameter(output, output_size, used, "softwaretype", software)) return false;
    if (observation.has_wind_direction &&
        !number(output, output_size, used, "winddir", observation.wind_direction_deg, 0)) return false;
    if (observation.has_wind_speed &&
        !number(output, output_size, used, "windspeedmph", observation.wind_speed_kmh * 0.621371f, 2)) return false;
    if (observation.has_wind_gust &&
        !number(output, output_size, used, "windgustmph", observation.wind_gust_10m_kmh * 0.621371f, 2)) return false;
    if (observation.has_temperature &&
        !number(output, output_size, used, "tempf", observation.temperature_c * 1.8f + 32.0f, 2)) return false;
    if (observation.has_humidity &&
        !number(output, output_size, used, "humidity", observation.humidity_pct, 1)) return false;
    if (observation.has_dew_point &&
        !number(output, output_size, used, "dewptf", observation.dew_point_c * 1.8f + 32.0f, 2)) return false;
    if (observation.has_pressure &&
        !number(output, output_size, used, "baromin", observation.pressure_hpa * 0.029529983f, 4)) return false;
    if (observation.has_rain_hour &&
        !number(output, output_size, used, "rainin", observation.rain_hour_mm / 25.4f, 3)) return false;
    if (observation.has_rain_today &&
        !number(output, output_size, used, "dailyrainin", observation.rain_today_mm / 25.4f, 3)) return false;
    return true;
}

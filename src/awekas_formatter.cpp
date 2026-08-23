#include "awekas_formatter.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

namespace {
bool safe_field(const char* value) {
    return value && value[0] && strchr(value, ';') == nullptr &&
           strchr(value, '\r') == nullptr && strchr(value, '\n') == nullptr;
}

bool append(char* output, size_t output_size, size_t& used, const char* value) {
    const size_t length = strlen(value);
    if (used + length >= output_size) return false;
    memcpy(output + used, value, length);
    used += length;
    output[used] = '\0';
    return true;
}

bool append_encoded(char* output, size_t output_size, size_t& used, const char* value) {
    static const char hex[] = "0123456789ABCDEF";
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
}

bool build_awekas_query(char* output,
                        size_t output_size,
                        const char* username,
                        const char* password_md5,
                        double latitude,
                        double longitude,
                        const char* firmware_version,
                        const WeatherObservation& observation) {
    if (!output || output_size == 0 || !safe_field(username) || !safe_field(password_md5) ||
        strlen(password_md5) != 32 || !safe_field(firmware_version) ||
        latitude < -90.0 || latitude > 90.0 || longitude < -180.0 || longitude > 180.0 ||
        observation.timestamp_utc <= 0) return false;

    struct tm utc;
    gmtime_r(&observation.timestamp_utc, &utc);
    char date[11], clock[6], temperature[20] = "", humidity[20] = "", pressure[20] = "";
    char rain[20] = "", wind[20] = "", direction[20] = "", gust[20] = "";
    char software[80], latitude_text[24], longitude_text[24];
    if (strftime(date, sizeof(date), "%d.%m.%Y", &utc) == 0 ||
        strftime(clock, sizeof(clock), "%H:%M", &utc) == 0) return false;
    if (observation.has_temperature) snprintf(temperature, sizeof(temperature), "%.2f", observation.temperature_c);
    if (observation.has_humidity) snprintf(humidity, sizeof(humidity), "%.0f", observation.humidity_pct);
    if (observation.has_pressure) snprintf(pressure, sizeof(pressure), "%.1f", observation.pressure_hpa);
    if (observation.has_rain_today) snprintf(rain, sizeof(rain), "%.2f", observation.rain_today_mm);
    if (observation.has_wind_speed) snprintf(wind, sizeof(wind), "%.2f", observation.wind_speed_kmh);
    if (observation.has_wind_direction) snprintf(direction, sizeof(direction), "%.0f", observation.wind_direction_deg);
    if (observation.has_wind_gust) snprintf(gust, sizeof(gust), "%.2f", observation.wind_gust_10m_kmh);
    snprintf(software, sizeof(software), "byte4geek_%s", firmware_version);
    snprintf(longitude_text, sizeof(longitude_text), "%.5f", longitude);
    snprintf(latitude_text, sizeof(latitude_text), "%.5f", latitude);

    const char* fields[25] = {
        username, password_md5, date, clock, temperature, humidity, pressure, rain,
        wind, direction, "", "", "", "en", "", gust,
        "", "", "", "", "", "", software, longitude_text, latitude_text
    };
    output[0] = '\0';
    size_t used = 0;
    if (!append(output, output_size, used, "?val=")) return false;
    for (size_t i = 0; i < 25; ++i) {
        if (i > 0 && !append(output, output_size, used, ";")) return false;
        if (i == 0) {
            if (!append_encoded(output, output_size, used, fields[i])) return false;
        } else if (!append(output, output_size, used, fields[i])) return false;
    }
    return true;
}

#include "cwop_formatter.h"

#include <math.h>
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

void coordinate(double decimal_degrees,
                bool latitude,
                char positive,
                char negative,
                char* output,
                size_t output_size) {
    const double absolute = fabs(decimal_degrees);
    int degrees = static_cast<int>(absolute);
    double minutes = (absolute - degrees) * 60.0;
    // Carry rounding at 59.995 minutes into the degree field.
    if (minutes >= 59.995) {
        ++degrees;
        minutes = 0.0;
    }
    snprintf(output, output_size, latitude ? "%02d%05.2f%c" : "%03d%05.2f%c",
             degrees, minutes, decimal_degrees >= 0.0 ? positive : negative);
}

int clamp_value(int value, int minimum, int maximum) {
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

void value_or_missing(char* output, size_t output_size, bool available,
                      int value, int minimum = 0, int maximum = 999) {
    if (!available) {
        snprintf(output, output_size, "...");
        return;
    }
    snprintf(output, output_size, "%03d", clamp_value(value, minimum, maximum));
}

int rounded(float value) {
    return static_cast<int>(floorf(value + 0.5f));
}

bool valid_station_id(const char* station_id) {
    const size_t length = station_id ? strlen(station_id) : 0;
    if (length == 0 || length > 9) return false;
    for (size_t i = 0; i < length; ++i) {
        const char c = station_id[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-')) return false;
    }
    return true;
}

bool safe_token(const char* token) {
    if (!token || token[0] == '\0') return false;
    return strchr(token, '\r') == nullptr && strchr(token, '\n') == nullptr &&
           strchr(token, ' ') == nullptr;
}
}

bool build_cwop_login(char* output,
                      size_t output_size,
                      const char* station_id,
                      const char* passcode,
                      const char* firmware_version) {
    if (!output || output_size == 0 || !valid_station_id(station_id) ||
        !safe_token(passcode) || !safe_token(firmware_version)) return false;
    int written = snprintf(output, output_size,
                           "user %s pass %s vers byte4geek-weatherstation %s\r\n",
                           station_id, passcode, firmware_version);
    return written > 0 && static_cast<size_t>(written) < output_size;
}

bool build_cwop_packet(char* output,
                       size_t output_size,
                       const char* station_id,
                       double latitude,
                       double longitude,
                       const char* firmware_version,
                       const WeatherObservation& observation) {
    if (!output || output_size == 0 || !valid_station_id(station_id) ||
        !safe_token(firmware_version) ||
        latitude < -90.0 || latitude > 90.0 ||
        longitude < -180.0 || longitude > 180.0 ||
        observation.timestamp_utc <= 0) {
        return false;
    }

    output[0] = '\0';
    size_t used = 0;
    char field[96];

    snprintf(field, sizeof(field), "%s>APRS,TCPIP*:", station_id);
    if (!append(output, output_size, used, field)) return false;

    struct tm utc;
    gmtime_r(&observation.timestamp_utc, &utc);
    snprintf(field, sizeof(field), "/%02d%02d%02dz", utc.tm_mday, utc.tm_hour, utc.tm_min);
    if (!append(output, output_size, used, field)) return false;

    char latitude_text[16];
    char longitude_text[16];
    coordinate(latitude, true, 'N', 'S', latitude_text, sizeof(latitude_text));
    coordinate(longitude, false, 'E', 'W', longitude_text, sizeof(longitude_text));
    snprintf(field, sizeof(field), "%s/%s_", latitude_text, longitude_text);
    if (!append(output, output_size, used, field)) return false;

    char direction[8], speed[8], gust[8], temperature[8];
    value_or_missing(direction, sizeof(direction), observation.has_wind_direction,
                     observation.has_wind_direction ? rounded(observation.wind_direction_deg) % 360 : 0);
    value_or_missing(speed, sizeof(speed), observation.has_wind_speed,
                     observation.has_wind_speed ? rounded(observation.wind_speed_kmh * 0.621371f) : 0);
    value_or_missing(gust, sizeof(gust), observation.has_wind_gust,
                     observation.has_wind_gust ? rounded(observation.wind_gust_10m_kmh * 0.621371f) : 0);
    value_or_missing(temperature, sizeof(temperature), observation.has_temperature,
                     observation.has_temperature ? rounded(observation.temperature_c * 1.8f + 32.0f) : 0,
                     -99, 999);
    snprintf(field, sizeof(field), "%s/%sg%st%s", direction, speed, gust, temperature);
    if (!append(output, output_size, used, field)) return false;

    char rain_hour[8], rain_24h[8], rain_today[8];
    value_or_missing(rain_hour, sizeof(rain_hour), observation.has_rain_hour,
                     observation.has_rain_hour ? rounded(observation.rain_hour_mm / 25.4f * 100.0f) : 0);
    value_or_missing(rain_24h, sizeof(rain_24h), observation.has_rain_24h,
                     observation.has_rain_24h ? rounded(observation.rain_24h_mm / 25.4f * 100.0f) : 0);
    value_or_missing(rain_today, sizeof(rain_today), observation.has_rain_today,
                     observation.has_rain_today ? rounded(observation.rain_today_mm / 25.4f * 100.0f) : 0);
    snprintf(field, sizeof(field), "r%sp%sP%s", rain_hour, rain_24h, rain_today);
    if (!append(output, output_size, used, field)) return false;

    if (observation.has_pressure) {
        snprintf(field, sizeof(field), "b%05d",
                 clamp_value(rounded(observation.pressure_hpa * 10.0f), 0, 99999));
    } else {
        snprintf(field, sizeof(field), "b.....");
    }
    if (!append(output, output_size, used, field)) return false;

    if (observation.has_humidity) {
        int humidity = clamp_value(rounded(observation.humidity_pct), 0, 100);
        snprintf(field, sizeof(field), "h%02d", humidity >= 100 ? 0 : humidity);
    } else {
        snprintf(field, sizeof(field), "h..");
    }
    if (!append(output, output_size, used, field)) return false;

    snprintf(field, sizeof(field), ".byte4geek-%s\r\n", firmware_version);
    return append(output, output_size, used, field);
}

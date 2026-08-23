#include "weathercloud_formatter.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main() {
    WeatherObservation observation = {};
    observation.timestamp_utc = 1700742300; // 2023-11-23 12:25 UTC
    observation.has_temperature = true;
    observation.has_humidity = true;
    observation.has_pressure = true;
    observation.has_dew_point = true;
    observation.has_wind_speed = true;
    observation.has_wind_direction = true;
    observation.has_wind_gust = true;
    observation.has_rain_today = true;
    observation.temperature_c = 22.2222f;
    observation.humidity_pct = 100.0f;
    observation.pressure_hpa = 1015.3f;
    observation.dew_point_c = 12.3f;
    observation.wind_speed_kmh = 8.04672f;
    observation.wind_direction_deg = 245.0f;
    observation.wind_gust_10m_kmh = 17.7028f;
    observation.rain_today_mm = 2.794f;

    char query[512];
    assert(build_weathercloud_query(query, sizeof(query), "device id", "secret/key",
                                    "1.0.4", observation));
    const char* expected =
        "?wid=device%20id&key=secret%2Fkey&date=20231123&time=1225"
        "&temp=222&hum=100&wdir=245&wspd=22&wspdhi=49&bar=10153&rain=28&dew=123&ver=1.0.4";
    if (strcmp(query, expected) != 0) fprintf(stderr, "actual: %s\n", query);
    assert(strcmp(query, expected) == 0);
    assert(!build_weathercloud_query(query, 12, "device", "key", "1.0.4", observation));
    return 0;
}

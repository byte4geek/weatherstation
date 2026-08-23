#include "windy_formatter.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main() {
    WeatherObservation observation = {};
    observation.timestamp_utc = 1700742300;
    observation.has_temperature = true;
    observation.has_dew_point = true;
    observation.has_humidity = true;
    observation.has_wind_speed = true;
    observation.has_wind_gust = true;
    observation.has_wind_direction = true;
    observation.has_pressure = true;
    observation.has_rain_hour = true;
    observation.temperature_c = 22.2222f;
    observation.dew_point_c = 12.3f;
    observation.humidity_pct = 55.1f;
    observation.wind_speed_kmh = 8.04672f;
    observation.wind_gust_10m_kmh = 17.7028f;
    observation.wind_direction_deg = 245.0f;
    observation.pressure_hpa = 1015.3f;
    observation.rain_hour_mm = 1.016f;

    char query[512];
    assert(build_windy_query(query, sizeof(query), "station/id", "1.0.4", observation));
    const char* expected =
        "?id=station%2Fid&ts=1700742300&temp=22.22&dewpoint=12.30&humidity=55.1"
        "&wind=2.24&gust=4.92&winddir=245&pressure=101530&precip=1.02"
        "&softwaretype=byte4geek-weatherstation-1.0.4";
    if (strcmp(query, expected) != 0) fprintf(stderr, "actual: %s\n", query);
    assert(strcmp(query, expected) == 0);
    return 0;
}

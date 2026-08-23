#include "awekas_formatter.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main() {
    WeatherObservation observation = {};
    observation.timestamp_utc = 1700742300;
    observation.has_temperature = true;
    observation.has_humidity = true;
    observation.has_pressure = true;
    observation.has_wind_speed = true;
    observation.has_wind_direction = true;
    observation.has_wind_gust = true;
    observation.has_rain_today = true;
    observation.temperature_c = 22.2222f;
    observation.humidity_pct = 55.1f;
    observation.pressure_hpa = 1015.3f;
    observation.wind_speed_kmh = 8.04672f;
    observation.wind_direction_deg = 245.0f;
    observation.wind_gust_10m_kmh = 17.7028f;
    observation.rain_today_mm = 2.794f;

    char query[640];
    const char* hash = "5f4dcc3b5aa765d61d8327deb882cf99";
    assert(build_awekas_query(query, sizeof(query), "station name", hash,
                              42.82618, -85.49441, "1.0.4", observation));
    const char* expected =
        "?val=station%20name;5f4dcc3b5aa765d61d8327deb882cf99;23.11.2023;12:25;"
        "22.22;55;1015.3;2.79;8.05;245;;;;en;;17.70;;;;;;;"
        "byte4geek_1.0.4;-85.49441;42.82618";
    if (strcmp(query, expected) != 0) fprintf(stderr, "actual: %s\n", query);
    assert(strcmp(query, expected) == 0);
    assert(!build_awekas_query(query, sizeof(query), "bad;name", hash,
                               42.0, -85.0, "1.0.4", observation));
    return 0;
}

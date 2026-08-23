#include "wow_be_formatter.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main() {
    WeatherObservation observation = {};
    observation.timestamp_utc = 1700742300;
    observation.has_temperature = true;
    observation.has_humidity = true;
    observation.has_pressure = true;
    observation.has_dew_point = true;
    observation.has_wind_speed = true;
    observation.has_wind_direction = true;
    observation.has_wind_gust = true;
    observation.has_rain_hour = true;
    observation.has_rain_today = true;
    observation.temperature_c = 22.2222f;
    observation.humidity_pct = 55.1f;
    observation.pressure_hpa = 1015.3f;
    observation.dew_point_c = 12.3f;
    observation.wind_speed_kmh = 8.04672f;
    observation.wind_direction_deg = 245.0f;
    observation.wind_gust_10m_kmh = 17.7028f;
    observation.rain_hour_mm = 1.016f;
    observation.rain_today_mm = 2.794f;

    char query[640];
    assert(build_wow_be_query(query, sizeof(query), "site id", "secret/key", "1.0.4", observation));
    const char* expected =
        "?action=updateraw&siteid=site%20id&siteAuthenticationKey=secret%2Fkey"
        "&dateutc=2023-11-23%2012%3A25%3A00&softwaretype=byte4geek-weatherstation-1.0.4"
        "&winddir=245&windspeedmph=5.00&windgustmph=11.00&tempf=72.00&humidity=55.1"
        "&dewptf=54.14&baromin=29.9818&rainin=0.040&dailyrainin=0.110";
    if (strcmp(query, expected) != 0) fprintf(stderr, "actual: %s\n", query);
    assert(strcmp(query, expected) == 0);
    return 0;
}

#include "cwop_formatter.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main() {
    WeatherObservation observation = {};
    observation.timestamp_utc = 1700742300; // 2023-11-23 12:25 UTC
    observation.has_temperature = true;
    observation.has_humidity = true;
    observation.has_pressure = true;
    observation.has_wind_speed = true;
    observation.has_wind_direction = true;
    observation.has_wind_gust = true;
    observation.has_rain_hour = true;
    observation.has_rain_24h = true;
    observation.has_rain_today = true;
    observation.temperature_c = 22.2222f; // 72 F
    observation.humidity_pct = 100.0f;
    observation.pressure_hpa = 1015.3f;
    observation.wind_speed_kmh = 8.04672f; // 5 mph
    observation.wind_direction_deg = 245.0f;
    observation.wind_gust_10m_kmh = 17.7028f; // 11 mph
    observation.rain_hour_mm = 1.016f; // 0.04 in
    observation.rain_24h_mm = 2.032f; // 0.08 in
    observation.rain_today_mm = 2.794f; // 0.11 in

    char login[128];
    assert(build_cwop_login(login, sizeof(login), "CW1234", "-1", "1.0.4"));
    assert(strcmp(login, "user CW1234 pass -1 vers byte4geek-weatherstation 1.0.4\r\n") == 0);
    assert(!build_cwop_login(login, sizeof(login), "CW12\n34", "-1", "1.0.4"));

    char packet[256];
    assert(build_cwop_packet(packet, sizeof(packet), "CW1234",
                             42.82618, -85.49441, "1.0.4", observation));
    const char* expected =
        "CW1234>APRS,TCPIP*:/231225z4249.57N/08529.66W_245/005g011t072r004p008P011b10153h00.byte4geek-1.0.4\r\n";
    if (strcmp(packet, expected) != 0) fprintf(stderr, "actual: %s", packet);
    assert(strcmp(packet, expected) == 0);

    assert(!build_cwop_packet(packet, sizeof(packet), "CW1234",
                              91.0, -85.0, "1.0.4", observation));
    return 0;
}

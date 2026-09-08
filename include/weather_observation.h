#pragma once

#include <stdint.h>
#include <time.h>

struct WeatherObservation {
    time_t timestamp_utc;

    bool has_temperature;
    bool has_humidity;
    bool has_pressure;
    bool has_dew_point;
    bool has_solar_radiation;
    bool has_wind_speed;
    bool has_wind_direction;
    bool has_wind_gust;
    bool has_rain_hour;
    bool has_rain_24h;
    bool has_rain_today;

    float temperature_c;
    float humidity_pct;
    float pressure_hpa;
    float dew_point_c;
    float solar_radiation_wm2;
    float wind_speed_kmh;
    float wind_direction_deg;
    float wind_gust_10m_kmh;
    float rain_hour_mm;
    float rain_24h_mm;
    float rain_today_mm;
};

struct WeatherObservationInput {
    time_t timestamp_utc;
    bool has_temperature;
    bool has_humidity;
    bool has_pressure;
    bool has_solar_radiation;
    bool has_wind_direction;
    float temperature_c;
    float humidity_pct;
    float pressure_hpa;
    float solar_radiation_wm2;
    float wind_speed_kmh;
    float wind_direction_deg;
    float wind_gust_10m_kmh;
    float rain_hour_mm;
    float rain_24h_mm;
    float rain_today_mm;
};

float calculate_dew_point_c(float temperature_c, float humidity_pct);
WeatherObservation make_weather_observation(const WeatherObservationInput& input);

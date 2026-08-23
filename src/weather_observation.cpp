#include "weather_observation.h"

#include <math.h>

namespace {
bool valid(float value) {
    return isfinite(value);
}
}

float calculate_dew_point_c(float temperature_c, float humidity_pct) {
    if (!valid(temperature_c) || !valid(humidity_pct) ||
        humidity_pct <= 0.0f || humidity_pct > 100.0f) {
        return NAN;
    }

    const float a = 17.625f;
    const float b = 243.04f;
    const float gamma = logf(humidity_pct / 100.0f) +
                        (a * temperature_c) / (b + temperature_c);
    return (b * gamma) / (a - gamma);
}

WeatherObservation make_weather_observation(const WeatherObservationInput& input) {
    WeatherObservation observation = {};
    observation.timestamp_utc = input.timestamp_utc;

    observation.has_temperature = input.has_temperature && valid(input.temperature_c);
    observation.has_humidity = input.has_humidity && valid(input.humidity_pct) &&
                               input.humidity_pct >= 0.0f && input.humidity_pct <= 100.0f;
    observation.has_pressure = input.has_pressure && valid(input.pressure_hpa) &&
                               input.pressure_hpa > 0.0f;
    observation.has_wind_speed = valid(input.wind_speed_kmh) && input.wind_speed_kmh >= 0.0f;
    observation.has_wind_direction = input.has_wind_direction &&
                                     valid(input.wind_direction_deg) &&
                                     input.wind_direction_deg >= 0.0f &&
                                     input.wind_direction_deg < 360.0f;
    observation.has_wind_gust = valid(input.wind_gust_10m_kmh) &&
                                input.wind_gust_10m_kmh >= 0.0f;
    observation.has_rain_hour = valid(input.rain_hour_mm) && input.rain_hour_mm >= 0.0f;
    observation.has_rain_today = valid(input.rain_today_mm) && input.rain_today_mm >= 0.0f;

    observation.temperature_c = input.temperature_c;
    observation.humidity_pct = input.humidity_pct;
    observation.pressure_hpa = input.pressure_hpa;
    observation.wind_speed_kmh = input.wind_speed_kmh;
    observation.wind_direction_deg = input.wind_direction_deg;
    observation.wind_gust_10m_kmh = input.wind_gust_10m_kmh;
    observation.rain_hour_mm = input.rain_hour_mm;
    observation.rain_today_mm = input.rain_today_mm;

    observation.dew_point_c = calculate_dew_point_c(input.temperature_c, input.humidity_pct);
    observation.has_dew_point = observation.has_temperature && observation.has_humidity &&
                                valid(observation.dew_point_c);
    return observation;
}

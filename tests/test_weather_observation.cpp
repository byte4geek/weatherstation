#include "weather_observation.h"

#include <assert.h>
#include <math.h>

static bool near(float actual, float expected, float tolerance) {
    return fabsf(actual - expected) <= tolerance;
}

int main() {
    assert(near(calculate_dew_point_c(20.0f, 50.0f), 9.26f, 0.05f));
    assert(isnan(calculate_dew_point_c(20.0f, 0.0f)));
    assert(isnan(calculate_dew_point_c(20.0f, 101.0f)));

    WeatherObservationInput input = {};
    input.timestamp_utc = 1700000001;
    input.has_temperature = true;
    input.has_humidity = true;
    input.has_pressure = true;
    input.has_wind_direction = true;
    input.temperature_c = 20.0f;
    input.humidity_pct = 50.0f;
    input.pressure_hpa = 1013.25f;
    input.wind_speed_kmh = 10.0f;
    input.wind_direction_deg = 359.0f;
    input.wind_gust_10m_kmh = 20.0f;
    input.rain_hour_mm = 1.2f;
    input.rain_24h_mm = 2.3f;
    input.rain_today_mm = 3.4f;

    WeatherObservation observation = make_weather_observation(input);
    assert(observation.has_temperature);
    assert(observation.has_dew_point);
    assert(observation.has_pressure);
    assert(observation.has_wind_direction);
    assert(near(observation.dew_point_c, 9.26f, 0.05f));

    input.humidity_pct = -1.0f;
    observation = make_weather_observation(input);
    assert(!observation.has_humidity);
    assert(!observation.has_dew_point);
    return 0;
}

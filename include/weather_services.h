#pragma once

#include <ArduinoJson.h>
#include "weather_service_features.h"

enum class WeatherServiceId {
#if WEATHER_UPLOAD_WUNDERGROUND
    Wunderground,
#endif
#if WEATHER_UPLOAD_PWSWEATHER
    PwsWeather,
#endif
#if WEATHER_UPLOAD_CWOP
    Cwop,
#endif
#if WEATHER_UPLOAD_WEATHERCLOUD
    Weathercloud,
#endif
#if WEATHER_UPLOAD_WINDY
    Windy,
#endif
#if WEATHER_UPLOAD_AWEKAS
    Awekas,
#endif
#if WEATHER_UPLOAD_WOW_BE
    WowBe
#endif
};

void setup_weather_services();
void handle_weather_services();
void record_weather_service_wind_sample(float instant_speed_kmh);
void append_weather_services_config(JsonDocument& doc, bool include_secrets);
void save_weather_services_config(JsonVariantConst config);
bool queue_weather_service_test(WeatherServiceId service);
void append_weather_services_status(JsonDocument& doc);

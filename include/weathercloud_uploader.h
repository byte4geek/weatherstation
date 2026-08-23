#pragma once

#include <Arduino.h>

#include "weather_observation.h"

struct WeathercloudUploadResult {
    bool success;
    int http_status;
    String message;
};

WeathercloudUploadResult upload_weathercloud_observation(
    const String& device_id,
    const String& device_key,
    const WeatherObservation& observation);

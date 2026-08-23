#pragma once

#include <Arduino.h>

#include "weather_observation.h"

struct AwekasUploadResult {
    bool success;
    int http_status;
    String message;
};

AwekasUploadResult upload_awekas_observation(const String& username,
                                             const String& password,
                                             float latitude,
                                             float longitude,
                                             const WeatherObservation& observation);

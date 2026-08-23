#pragma once

#include <Arduino.h>

#include "weather_observation.h"

struct WindyUploadResult {
    bool success;
    int http_status;
    String message;
};

WindyUploadResult upload_windy_observation(const String& station_id,
                                           const String& station_password,
                                           const WeatherObservation& observation);

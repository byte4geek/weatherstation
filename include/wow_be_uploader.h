#pragma once

#include <Arduino.h>

#include "weather_observation.h"

struct WowBeUploadResult {
    bool success;
    int http_status;
    String message;
};

WowBeUploadResult upload_wow_be_observation(const String& site_id,
                                            const String& authentication_key,
                                            const WeatherObservation& observation);

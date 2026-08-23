#pragma once

#include <Arduino.h>

#include "weather_observation.h"

struct CwopUploadResult {
    bool success;
    String message;
};

CwopUploadResult upload_cwop_observation(const String& server,
                                         uint16_t port,
                                         const String& station_id,
                                         const String& passcode,
                                         double latitude,
                                         double longitude,
                                         const WeatherObservation& observation);

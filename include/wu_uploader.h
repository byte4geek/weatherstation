#pragma once

#include <Arduino.h>
#include "weather_observation.h"

struct WuUploadResult {
    bool success;
    int http_status;
    String message;
};

String url_encode(const String& value);
String build_wu_query(const String& station_id,
                      const String& station_key,
                      const WeatherObservation& observation);
WuUploadResult upload_wu_observation(const char* service_name,
                                     const char* base_url,
                                     const String& station_id,
                                     const String& station_key,
                                     const WeatherObservation& observation);
